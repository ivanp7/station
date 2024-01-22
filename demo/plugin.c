#include "plugin.h"

#include <station/plugin.typ.h>
#include <station/buffer.typ.h>

#include <station/signal.typ.h>
#include <station/signal.def.h>

#include <station/parallel.fun.h>
#include <station/sdl.fun.h>
#include <station/font.fun.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for alarm()


// Parallel processing function
static STATION_PFUNC(pfunc_inc) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    // Increment the counter safely
    mtx_lock(&resources->counter_mutex);
    resources->counter += task_idx;
    mtx_unlock(&resources->counter_mutex);
}

// Parallel processing function
static STATION_PFUNC(pfunc_dec) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    // Decrement the counter safely
    mtx_lock(&resources->counter_mutex);
    resources->counter -= task_idx;
    mtx_unlock(&resources->counter_mutex);
}

#ifdef STATION_IS_SDL_SUPPORTED
// Parallel processing function
static STATION_PFUNC(pfunc_draw) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    // Compute texture coordinates of the current pixel
    station_task_idx_t y = resources->sdl_window.texture.lock.rectangle.y +
        task_idx / resources->sdl_window.texture.lock.rectangle.width;
    station_task_idx_t x = resources->sdl_window.texture.lock.rectangle.x +
        task_idx % resources->sdl_window.texture.lock.rectangle.width;

    // Generate a simple animation
    uint32_t pixel = ((x+y) + resources->frame) & 0xFF;
    pixel = 0xFF000000 | (pixel << 16) | (pixel << 8) | pixel;

    // Update the texture
    resources->sdl_window.texture.lock.pixels[task_idx] = pixel;
}
#endif


// State function for the finite state machine
static STATION_SFUNC(sfunc_pre) // implicit arguments: state, fsm_data
{
    printf("sfunc_pre()\n");

    struct plugin_resources *resources = fsm_data;

    // Increment the counter to check if all task indices were processed
    station_parallel_processing_execute(resources->parallel_processing_context,
            pfunc_inc, resources, NUM_TASKS, BATCH_SIZE);

    // Sum of [0; N-1] is N*(N-1)/2
    if (resources->counter * 2 != (NUM_TASKS * (NUM_TASKS - 1)))
    {
        printf("counter has incorrect value\n");
        exit(1);
    }

    state->sfunc = sfunc_loop;
}

// State function for the finite state machine
static STATION_SFUNC(sfunc_loop) // implicit arguments: state, fsm_data
{
    struct plugin_resources *resources = fsm_data;
    station_signal_set_t *signals = resources->signals;

    // Check if caught any of the signals
    if (STATION_SIGNAL_IS_FLAG_SET(&signals->signal_SIGINT))
    {
        printf("Caught SIGINT, bye!\n");
        // Exit normally
        state->sfunc = sfunc_post;
        return;
    }
    else if (STATION_SIGNAL_IS_FLAG_SET(&signals->signal_SIGQUIT))
    {
        printf("Caught SIGQUIT, bye!\n");
        // Exit by quick_exit()
        quick_exit(EXIT_SUCCESS);
    }
    else if (STATION_SIGNAL_IS_FLAG_SET(&signals->signal_SIGTERM))
    {
        printf("Caught SIGTERM, bye!\n");
        // Exit by exit()
        exit(EXIT_SUCCESS);
    }

    if (STATION_SIGNAL_IS_FLAG_SET(&signals->signal_SIGTSTP))
    {
        printf("Caught SIGTSTP.\n");
        // Unset the signal flag
        STATION_SIGNAL_UNSET_FLAG(&signals->signal_SIGTSTP);

        if (!resources->alarm_set)
        {
            // Schedule SIGALRM and prepare to compute FPS
            printf("Setting an alarm in %d seconds.\n", ALARM_DELAY);
            alarm(ALARM_DELAY);

            resources->alarm_set = true;
            resources->prev_frame = resources->frame;
        }
    }

    if (STATION_SIGNAL_IS_FLAG_SET(&signals->signal_SIGALRM))
    {
        printf("Caught SIGALRM.\n");
        // Unset the signal flag
        STATION_SIGNAL_UNSET_FLAG(&signals->signal_SIGALRM);

        if (resources->alarm_set)
        {
            // Compute FPS
            printf("fps = %.2g\n", 1.0f * (resources->frame - resources->prev_frame) / ALARM_DELAY);
            resources->alarm_set = false;
        }
    }

    // Increase the frame counter
    resources->frame++;

    // Proceed to sfunc_loop_sdl(), unless SDL is disabled
#ifdef STATION_IS_SDL_SUPPORTED
    if (resources->sdl_window_created)
        state->sfunc = sfunc_loop_sdl;
#endif
}

#ifdef STATION_IS_SDL_SUPPORTED
// State function for the finite state machine
static STATION_SFUNC(sfunc_loop_sdl) // implicit arguments: state, fsm_data
{
    struct plugin_resources *resources = fsm_data;

    if (resources->sdl_window_created)
    {
        // Process SDL events
        while (SDL_PollEvent(&resources->event))
        {
            if (resources->event.type == SDL_QUIT)
            {
                printf("Window is closed, bye!\n");
                state->sfunc = sfunc_post;
                return;
            }
            else if ((resources->event.type == SDL_KEYDOWN) &&
                    (resources->event.key.keysym.sym == SDLK_ESCAPE))
            {
                printf("Escape is pressed, bye!\n");
                state->sfunc = sfunc_post;
                return;
            }
            else if ((resources->event.type == SDL_KEYDOWN) &&
                    (resources->event.key.keysym.sym == SDLK_SPACE))
                resources->window_frozen = !resources->window_frozen;
        }

        if (!resources->window_frozen)
        {
            // Update window texture

            // step 1: lock the texture area
            if (station_sdl_window_lock_texture(&resources->sdl_window,
                        true, 0, 0, 0, 0) != 0) // whole texture
            {
                printf("station_sdl_window_lock_texture() failure\n");
                state->sfunc = sfunc_post;
                return;
            }

            // step 2: update texture pixels by calling pfunc_draw() from multiple threads
            station_parallel_processing_execute(resources->parallel_processing_context,
                    pfunc_draw, resources, TEXTURE_WIDTH*TEXTURE_HEIGHT, BATCH_SIZE);

            // step 3: if have font and text, draw floating text
            if ((resources->font != NULL) && (resources->text != NULL))
            {
                const char *str = resources->text;
                int x, y;
                {
                    // Calculate number of UTF-8 characters in the string
                    int len = 0;
                    while (*str != '\0')
                    {
                        size_t chr_len;
                        station_font_psf2_glyph(str, strlen(str), &chr_len, resources->font);
                        str += chr_len;
                        len++;
                    }

                    // Calculate position of the text in the texture
                    x = (TEXTURE_WIDTH - len * (int)resources->font->header->width) / 2;
                    y = (TEXTURE_HEIGHT - (int)resources->font->header->height) / 2;
                }

                // For each UTF-8 character in the string:
                int i = 0;
                str = resources->text;
                while (*str != '\0')
                {
                    // Extract the glyph of the currect UTF-8 character
                    size_t chr_len;
                    const unsigned char *glyph = station_font_psf2_glyph(str, strlen(str),
                            &chr_len, resources->font);
                    str += chr_len;

                    if (glyph != NULL)
                    {
                        // Draw the glyph
                        station_sdl_window_texture_draw_glyph(
                                &resources->sdl_window,
                                x, y + (int)resources->font->header->height *
                                cos(1.0 * (resources->frame / 128.0f + i * M_PI/8)),
                                true, false,
                                0xFF0000FF, 0xFF888888,
                                glyph, resources->font->header->width, resources->font->header->height,
                                0, // resources->font->header->width-1,
                                0, // resources->font->header->height-1,
                                +(int)resources->font->header->width,
                                +(int)resources->font->header->height);
                    }

                    x += resources->font->header->width;
                    i++;
                }
            }

            // step 4: unlock the texture and render it
            if (station_sdl_window_unlock_texture_and_render(&resources->sdl_window) != 0)
            {
                printf("station_sdl_window_unlock_texture_and_render() failure\n");
                state->sfunc = sfunc_post;
                return;
            }
        }
    }

    // Proceed to sfunc_loop()
    state->sfunc = sfunc_loop;
}
#endif

// State function for the finite state machine
static STATION_SFUNC(sfunc_post) // implicit arguments: state, fsm_data
{
    printf("sfunc_post()\n");

    struct plugin_resources *resources = fsm_data;

    // Decrement the counter back to zero to become twice as sure
    station_parallel_processing_execute(resources->parallel_processing_context,
            pfunc_dec, resources, NUM_TASKS, BATCH_SIZE);

    if (resources->counter != 0)
    {
        printf("counter has incorrect value\n");
        exit(1);
    }

    // Stop the finite state machine
    state->sfunc = NULL;
}


// Plugin help function
static STATION_PLUGIN_HELP_FUNC(plugin_help) // implicit arguments: argc, argv
{
    printf("plugin_help(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    printf("Provide a font as the first --file,\n");
    printf("  give a string as a first plugin argument,\n");
    printf("  and voila -- observe a floating text!\n");
}

// Plugin configuration function
static STATION_PLUGIN_CONF_FUNC(plugin_conf) // implicit arguments: args, argc, argv
{
    printf("plugin_conf(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    if (argc >= 2)
        args->cmdline = argv[1]; // first argument will be draw as a floating text

    // Catch the following signals
    args->signals->signal_SIGINT = true;
    args->signals->signal_SIGQUIT = true;
    args->signals->signal_SIGTERM = true;

    args->opencl_is_used = true; // allow contexts to be created

    args->sdl_is_used = true;
#ifdef STATION_IS_SDL_SUPPORTED
    args->sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#endif
}

// Plugin initialization function
static STATION_PLUGIN_INIT_FUNC(plugin_init) // implicit arguments: inputs, outputs
{
    printf("plugin_init()\n");

    struct plugin_resources *resources = malloc(sizeof(*resources));
    if (resources == NULL)
    {
        printf("malloc() failed\n");
        exit(1);
    }

    outputs->plugin_resources = resources;

    outputs->fsm_initial_state.sfunc = sfunc_pre; // begin FSM execution from sfunc_pre()
    outputs->fsm_data = resources;

    resources->signals = inputs->signals;

    resources->parallel_processing_context = inputs->parallel_processing_context;

    if (inputs->sdl_is_available)
    {
        // Create a window
        station_sdl_window_properties_t properties = {
            .texture = {.width = TEXTURE_WIDTH, .height = TEXTURE_HEIGHT},
            .window = {.width = TEXTURE_WIDTH * WINDOW_SCALE,
                .height = TEXTURE_HEIGHT * WINDOW_SCALE,
                .title = "Demo window"},
        };

        int code = station_sdl_initialize_window_context(&resources->sdl_window, &properties);
        if (code != 0)
        {
            printf("station_sdl_initialize_window_context() returned %i\n", code);
            free(resources);
            exit(1);
        }

        resources->sdl_window_created = true;
    }
    else
        resources->sdl_window_created = false;

    resources->window_frozen = false;

    if (inputs->files->num_buffers > 0)
    {
        // Load a font
        resources->font = station_load_font_psf2_from_buffer(
                &inputs->files->buffers[0]);

        if (resources->font == NULL)
            printf("Couldn't load PSFv2 font from file #0\n");
        else
            printf("Font size (WxH): %ux%u\n", resources->font->header->width,
                    resources->font->header->height);
    }
    else
        resources->font = NULL;

    resources->text = inputs->cmdline;

    // Initialize a counter for parallel processing test
    resources->counter = 0;
    mtx_init(&resources->counter_mutex, mtx_plain);

    // Other variables
    resources->alarm_set = false;
    resources->prev_frame = 0;
    resources->frame = 0;
}

// Plugin finalization function
static STATION_PLUGIN_FINAL_FUNC(plugin_final) // implicit arguments: plugin_resources, quick
{
    printf("plugin_final()\n");

    struct plugin_resources *resources = plugin_resources;

    if (!quick)
    {
        mtx_destroy(&resources->counter_mutex);

        station_unload_font_psf2(resources->font);
    }

    if (resources->sdl_window_created)
        station_sdl_destroy_window_context(&resources->sdl_window);

    if (!quick)
        free(resources);

    return 0; // success
}

// Define the plugin
STATION_PLUGIN("Demo plugin", plugin_help, plugin_conf, plugin_init, plugin_final)


// Make the plugin executable without station-app because we can
#ifdef __GNUC__

void plugin_main(void);

__attribute__((force_align_arg_pointer))
void plugin_main(void)
{
    printf("Demo plugin for station-app\n");
    exit(0);
}

const char dl_loader[] __attribute__((section(".interp"))) = DL_LOADER;

#endif

