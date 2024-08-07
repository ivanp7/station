#include "plugin.h"

#include <station/plugin.typ.h>
#include <station/buffer.typ.h>

#include <station/signal.typ.h>

#include <station/concurrent.fun.h>
#include <station/sdl.fun.h>
#include <station/buffer.fun.h>
#include <station/font.fun.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <signal.h>
#include <unistd.h> // for alarm()


// Signal handler function
static STATION_SIGNAL_HANDLER_FUNC(signal_handler) // implicit arguments: signo, siginfo, std_signals, rt_signals, data
{
    (void) siginfo;
    (void) std_signals;
    (void) rt_signals;
    (void) data;

    if ((signo >= SIGRTMIN) && (signo <= SIGRTMAX))
    {
        if (signo <= SIGRTMIN + (SIGRTMAX-SIGRTMIN)/2)
            printf("Caught real-time signal SIGRTMIN%+i!\n", signo - SIGRTMIN);
        else
            printf("Caught real-time signal SIGRTMAX%+i!\n", signo - SIGRTMAX);
    }

    return true;
}

// Concurrent processing callback function
static STATION_PFUNC_CALLBACK(pfunc_cb_flag) // implicit arguments: data, thread_idx
{
    (void) thread_idx;

    atomic_bool *flag = data;
    *flag = true;
}

// Concurrent processing function
static STATION_PFUNC(pfunc_inc) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    // Increment the counter safely
    mtx_lock(&resources->counter_mutex);
    resources->counter += task_idx;
    mtx_unlock(&resources->counter_mutex);
}

// Concurrent processing function
static STATION_PFUNC(pfunc_dec) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    // Decrement the counter safely
    mtx_lock(&resources->counter_mutex);
    resources->counter -= task_idx;
    mtx_unlock(&resources->counter_mutex);
}

// Concurrent processing function
static STATION_PFUNC(pfunc_queue) // implicit arguments: data, task_idx, thread_idx
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    while (!station_queue_push(resources->queue, &task_idx)); // wait until push
    task_idx = 0;
    while (!station_queue_pop(resources->queue, &task_idx)); // wait until pop

    // Increment the counter safely
    mtx_lock(&resources->counter_mutex);
    resources->counter += task_idx;
    mtx_unlock(&resources->counter_mutex);
}

#ifdef STATION_IS_SDL_SUPPORTED
// Concurrent processing function
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
    printf("\nsfunc_pre()\n");

    struct plugin_resources *resources = fsm_data;

    if (resources->concurrent_processing_context != NULL)
    {
        atomic_bool flag = false;
        bool result;

        // Stress test of concurrent processing
        printf("Performing stress-test of concurrent processing...\n");

        for (unsigned i = 0; i < NUM_ITERATIONS; i++)
        {
            // Increment the counter to check if all task indices were processed
            do
            {
                result = station_concurrent_processing_execute(resources->concurrent_processing_context,
                        NUM_TASKS, BATCH_SIZE, pfunc_inc, resources,
                        pfunc_cb_flag, &flag, false); // non-blocking call
                        /* NULL, &flag, false); // blocking call */
            }
            while (!result);

            // Busy-wait until done
            while (!flag);
            flag = false;

            // Sum of [0; N-1] is N*(N-1)/2
            if (resources->counter * 2 != (NUM_TASKS * (NUM_TASKS - 1)))
            {
                printf("counter has incorrect value\n");
                exit(1);
            }

            // Decrement the counter back to zero to become twice as sure
            do
            {
                result = station_concurrent_processing_execute(resources->concurrent_processing_context,
                        NUM_TASKS, BATCH_SIZE, pfunc_dec, resources,
                        pfunc_cb_flag, &flag, false); // non-blocking call
                        /* NULL, &flag, false); // blocking call */
            }
            while (!result);

            // Busy-wait until done
            while (!flag);
            flag = false;

            // Counter must be equal to zero again
            if (resources->counter != 0)
            {
                printf("counter is not 0\n");
                exit(1);
            }
        }

        if (resources->queue != NULL)
        {
            printf("Performing stress-test of lock-free queue...\n");

            for (unsigned i = 0; i < NUM_ITERATIONS; i++)
            {
                // Increment the counter to check if all task indices were processed
                do
                {
                    result = station_concurrent_processing_execute(resources->concurrent_processing_context,
                            NUM_TASKS, BATCH_SIZE, pfunc_queue, resources,
                            NULL, &flag, false); // blocking call
                }
                while (!result);

                // Sum of [0; N-1] is N*(N-1)/2
                if (resources->counter * 2 != (NUM_TASKS * (NUM_TASKS - 1)))
                {
                    printf("counter has incorrect value\n");
                    exit(1);
                }

                resources->counter = 0;
            }
        }

        printf("Stress-test is complete!\n");
    }

    state->sfunc = sfunc_loop;
}

// State function for the finite state machine
static STATION_SFUNC(sfunc_loop) // implicit arguments: state, fsm_data
{
    struct plugin_resources *resources = fsm_data;

    // Check if caught any of the signals
    if (STATION_SIGNAL_IS_FLAG_SET(&resources->std_signals->signal_SIGINT))
    {
        printf("Caught SIGINT, bye!\n");
        // Exit normally
        state->sfunc = NULL;
        return;
    }
    else if (STATION_SIGNAL_IS_FLAG_SET(&resources->std_signals->signal_SIGQUIT))
    {
        printf("Caught SIGQUIT, bye!\n");
        // Exit by quick_exit()
        quick_exit(EXIT_SUCCESS);
    }
    else if (STATION_SIGNAL_IS_FLAG_SET(&resources->std_signals->signal_SIGTERM))
    {
        printf("Caught SIGTERM, bye!\n");
        // Exit by exit()
        exit(EXIT_SUCCESS);
    }

    if (STATION_SIGNAL_IS_FLAG_SET(&resources->std_signals->signal_SIGTSTP))
    {
        printf("Caught SIGTSTP.\n");
        // Unset the signal flag
        STATION_SIGNAL_UNSET_FLAG(&resources->std_signals->signal_SIGTSTP);

        if (!resources->alarm_set)
        {
            // Schedule SIGALRM and prepare to compute FPS
            printf("Setting an alarm in %d seconds.\n", ALARM_DELAY);
            alarm(ALARM_DELAY);

            resources->alarm_set = true;
            resources->prev_frame = resources->frame;
        }
    }

    if (STATION_SIGNAL_IS_FLAG_SET(&resources->std_signals->signal_SIGALRM))
    {
        printf("Caught SIGALRM.\n");
        // Unset the signal flag
        STATION_SIGNAL_UNSET_FLAG(&resources->std_signals->signal_SIGALRM);

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
                exit(EXIT_SUCCESS);
            }
            else if ((resources->event.type == SDL_KEYDOWN) &&
                    (resources->event.key.keysym.sym == SDLK_ESCAPE))
            {
                printf("Escape is pressed, bye!\n");
                state->sfunc = NULL;
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
                exit(EXIT_FAILURE);
            }

            // step 2: update texture pixels by calling pfunc_draw() from multiple threads
            if (resources->concurrent_processing_context != NULL)
                station_concurrent_processing_execute(resources->concurrent_processing_context,
                        TEXTURE_WIDTH*TEXTURE_HEIGHT, BATCH_SIZE, pfunc_draw, resources,
                        NULL, NULL, resources->concurrent_processing_context->busy_wait); // blocking call
            else
                for (station_task_idx_t task_idx = 0; task_idx < TEXTURE_WIDTH*TEXTURE_HEIGHT; task_idx++)
                    pfunc_draw(resources, task_idx, 0);

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
                exit(EXIT_FAILURE);
            }
        }
    }

    // Proceed to sfunc_loop()
    state->sfunc = sfunc_loop;
}
#endif

///////////////////////////////////////////////////////////////////////

// Plugin help function
static STATION_PLUGIN_HELP_FUNC(plugin_help) // implicit arguments: argc, argv
{
    printf("\nplugin_help(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    printf("\nProvide a font as the first --file,\n");
    printf("  give a string as a first plugin argument,\n");
    printf("  and voila -- observe a floating text!\n");
}

// Plugin configuration function
static STATION_PLUGIN_CONF_FUNC(plugin_conf) // implicit arguments: args, argc, argv
{
    printf("\nplugin_conf(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    if (argc >= 2)
        args->cmdline = argv[1]; // first argument will be draw as a floating text

    // Catch the following signals
    args->signal_handler = signal_handler;

    args->num_files_used = 1;
    args->num_concurrent_processing_contexts_used = 1;
    args->num_opencl_contexts_used = -1; // allow any number of contexts to be created
    args->sdl_is_used = true;

#ifdef STATION_IS_SDL_SUPPORTED
    args->sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#endif
}

// Plugin initialization function
static STATION_PLUGIN_INIT_FUNC(plugin_init) // implicit arguments: inputs, outputs
{
    printf("\nplugin_init()\n");

    struct plugin_resources *resources = malloc(sizeof(*resources));
    if (resources == NULL)
    {
        printf("malloc() failed\n");
        exit(1);
    }

    *resources = (struct plugin_resources){0};

    outputs->plugin_resources = resources;

    outputs->fsm_initial_state.sfunc = sfunc_pre; // begin FSM execution from sfunc_pre()
    outputs->fsm_data = resources;

    resources->std_signals = inputs->std_signals;
    resources->rt_signals = inputs->rt_signals;

    if (inputs->concurrent_processing_contexts->num_contexts > 0)
        resources->concurrent_processing_context = &inputs->concurrent_processing_contexts->contexts[0];
    else
        resources->concurrent_processing_context = NULL;

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

    if (inputs->num_files > 0)
    {
        // Load a font
        if (station_buffer_read_whole_file(&resources->font_buffer, inputs->files[0]))
        {
            resources->font = station_load_font_psf2_from_buffer(&resources->font_buffer);

            if (resources->font != NULL)
            {
                printf("Font size (WxH): %ux%u\n", resources->font->header->width,
                        resources->font->header->height);

                station_buffer_resize(&resources->font_buffer,
                        station_font_psf2_glyph_data_size(resources->font->header));
            }
            else
                printf("Couldn't load PSFv2 font from file #0\n");
        }
        else
            printf("Couldn't read PSFv2 font from file #0\n");
    }
    else
        resources->font = NULL;

    resources->text = inputs->cmdline;

    // Initialize a counter for concurrent processing test
    resources->counter = 0;
    mtx_init(&resources->counter_mutex, mtx_plain);

    // Create lock-free queue for stress-test
    resources->queue = station_create_queue(sizeof(station_task_idx_t),
            QUEUE_ALIGNMENT_LOG2, QUEUE_CAPACITY_LOG2);

    // Other variables
    resources->alarm_set = false;
    resources->prev_frame = 0;
    resources->frame = 0;
}

// Plugin finalization function
static STATION_PLUGIN_FINAL_FUNC(plugin_final) // implicit arguments: plugin_resources, quick
{
    printf("\nplugin_final()\n");

    struct plugin_resources *resources = plugin_resources;

    if (!quick)
    {
        mtx_destroy(&resources->counter_mutex);
        station_destroy_queue(resources->queue);

        station_unload_font_psf2(resources->font);
        station_buffer_clear(&resources->font_buffer);
    }

    if (resources->sdl_window_created)
        station_sdl_destroy_window_context(&resources->sdl_window);

    if (!quick)
        free(resources);

    return 0; // success
}

// Define the plugin
STATION_PLUGIN("demo", "Demo plugin", plugin_help, plugin_conf, plugin_init, plugin_final);

///////////////////////////////////////////////////////////////////////

// Make the plugin shared library executable because we can
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

