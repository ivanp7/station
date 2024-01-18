#include "plugin.h"

#include <station/plugin.typ.h>
#include <station/signal.typ.h>
#include <station/buffer.typ.h>

#include <station/parallel.fun.h>
#include <station/sdl.fun.h>
#include <station/font.fun.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static STATION_PFUNC(pfunc_inc)
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    mtx_lock(&resources->counter_mutex);
    resources->counter += task_idx;
    mtx_unlock(&resources->counter_mutex);
}

static STATION_PFUNC(pfunc_dec)
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    mtx_lock(&resources->counter_mutex);
    resources->counter -= task_idx;
    mtx_unlock(&resources->counter_mutex);
}

#ifdef STATION_IS_SDL_SUPPORTED
static STATION_PFUNC(pfunc_draw)
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    station_task_idx_t y = task_idx / TEXTURE_WIDTH;
    station_task_idx_t x = task_idx % TEXTURE_WIDTH;

    uint32_t pixel = ((x+y) + resources->frame) & 0xFF;
    pixel = 0xFF000000 | (pixel << 16) | (pixel << 8) | pixel;

    resources->sdl_window.texture.lock.pixels[task_idx] = pixel;
}
#endif


static STATION_SFUNC(sfunc_pre)
{
    printf("sfunc_pre()\n");

    struct plugin_resources *resources = fsm_data;

    station_parallel_processing_execute(resources->parallel_processing_context,
            pfunc_inc, resources, NUM_TASKS, BATCH_SIZE);

    if (resources->counter * 2 != (NUM_TASKS * (NUM_TASKS - 1)))
    {
        printf("counter has incorrect value\n");
        exit(1);
    }

    state->sfunc = sfunc_loop;
}

static STATION_SFUNC(sfunc_loop)
{
    struct plugin_resources *resources = fsm_data;
    station_signal_set_t *signals = resources->signals;

    if (atomic_load_explicit(&signals->signal_SIGINT, memory_order_acquire))
    {
        printf("Caught SIGINT, bye!\n");
        state->sfunc = sfunc_post;
        return;
    }
    else if (atomic_load_explicit(&signals->signal_SIGQUIT, memory_order_acquire))
    {
        printf("Caught SIGQUIT, bye!\n");
        quick_exit(EXIT_SUCCESS);
    }
    else if (atomic_load_explicit(&signals->signal_SIGTERM, memory_order_acquire))
    {
        printf("Caught SIGTERM, bye!\n");
        exit(EXIT_SUCCESS);
    }

    if (atomic_load_explicit(&signals->signal_SIGTSTP, memory_order_acquire))
    {
        printf("Caught SIGTSTP.\n");
        atomic_store_explicit(&signals->signal_SIGTSTP, false, memory_order_release);

        if (!resources->alarm_set)
        {
            printf("Setting an alarm in %d seconds.\n", ALARM_DELAY);
            alarm(ALARM_DELAY);

            resources->alarm_set = true;
            resources->prev_frame = resources->frame;
        }
    }

    if (atomic_load_explicit(&signals->signal_SIGALRM, memory_order_acquire))
    {
        printf("Caught SIGALRM.\n");
        atomic_store_explicit(&signals->signal_SIGALRM, false, memory_order_release);

        if (resources->alarm_set)
        {
            printf("fps = %.2g\n", 1.0f * (resources->frame - resources->prev_frame) / ALARM_DELAY);
            resources->alarm_set = false;
        }
    }

    resources->frame++;

#ifdef STATION_IS_SDL_SUPPORTED
    if (resources->sdl_window_created)
        state->sfunc = sfunc_loop_sdl;
#endif
}

#ifdef STATION_IS_SDL_SUPPORTED
static STATION_SFUNC(sfunc_loop_sdl)
{
    struct plugin_resources *resources = fsm_data;

    if (resources->sdl_window_created)
    {
        SDL_PollEvent(&resources->event);

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
            resources->frozen = !resources->frozen;

        if (!resources->frozen)
        {
            if (station_sdl_window_lock_texture(&resources->sdl_window,
                        true, 0, 0, 0, 0) != 0)
            {
                printf("station_sdl_window_lock_texture() failure\n");
                state->sfunc = sfunc_post;
                return;
            }

            station_parallel_processing_execute(resources->parallel_processing_context,
                    pfunc_draw, resources, TEXTURE_WIDTH*TEXTURE_HEIGHT, BATCH_SIZE);

            if ((resources->font != NULL) && (resources->text != NULL))
            {
                const char *str = resources->text;
                int x, y;
                {
                    int len = 0;
                    while (*str != '\0')
                    {
                        size_t chr_len;
                        station_font_psf2_glyph(str, strlen(str), &chr_len, resources->font);
                        str += chr_len;
                        len++;
                    }

                    x = (TEXTURE_WIDTH - len * (int)resources->font->header->width) / 2;
                    y = (TEXTURE_HEIGHT - (int)resources->font->header->height) / 2;
                }

                int i = 0;
                str = resources->text;
                while (*str != '\0')
                {
                    size_t chr_len;
                    const unsigned char *glyph = station_font_psf2_glyph(str, strlen(str),
                            &chr_len, resources->font);
                    str += chr_len;

                    if (glyph != NULL)
                    {
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

            if (station_sdl_window_unlock_texture_and_render(&resources->sdl_window) != 0)
            {
                printf("station_sdl_window_unlock_texture_and_render() failure\n");
                state->sfunc = sfunc_post;
                return;
            }
        }
    }

    state->sfunc = sfunc_loop;
}
#endif

static STATION_SFUNC(sfunc_post)
{
    printf("sfunc_post()\n");

    struct plugin_resources *resources = fsm_data;

    station_parallel_processing_execute(resources->parallel_processing_context,
            pfunc_dec, resources, NUM_TASKS, BATCH_SIZE);

    if (resources->counter != 0)
    {
        printf("counter has incorrect value\n");
        exit(1);
    }

    state->sfunc = NULL;
}


static STATION_PLUGIN_HELP_FUNC(plugin_help)
{
    printf("plugin_help(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");
}

static STATION_PLUGIN_CONF_FUNC(plugin_conf)
{
    printf("plugin_conf(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    if (argc >= 2)
        args->cmdline = argv[1];

    args->signals->signal_SIGINT = true;
    args->signals->signal_SIGQUIT = true;
    args->signals->signal_SIGTERM = true;

    args->opencl_is_used = true; // allow contexts to be created

    args->sdl_is_used = true;
#ifdef STATION_IS_SDL_SUPPORTED
    args->sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#endif
}

static STATION_PLUGIN_INIT_FUNC(plugin_init)
{
    printf("plugin_init()\n");

    struct plugin_resources *resources = malloc(sizeof(*resources));
    if (resources == NULL)
    {
        printf("malloc() failed\n");
        exit(1);
    }

    outputs->plugin_resources = resources;

    outputs->fsm_initial_state.sfunc = sfunc_pre;
    outputs->fsm_data = resources;

    resources->signals = inputs->signals;

    resources->parallel_processing_context = inputs->parallel_processing_context;

    if (inputs->sdl_is_available)
    {
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

    if (inputs->files->num_buffers > 0)
    {
        resources->font = station_load_font_psf2_from_buffer(
                inputs->files->buffers[0]);

        if (resources->font == NULL)
            printf("Couldn't load PSFv2 font from file #0\n");
    }
    else
        resources->font = NULL;

    if (inputs->cmdline != NULL)
        resources->text = inputs->cmdline;

    resources->counter = 0;
    mtx_init(&resources->counter_mutex, mtx_plain);

    resources->frozen = false;
    resources->alarm_set = false;
    resources->prev_frame = 0;
    resources->frame = 0;
}

static STATION_PLUGIN_FINAL_FUNC(plugin_final)
{
    (void) quick;

    printf("plugin_final()\n");

    struct plugin_resources *resources = plugin_resources;

    if (resources != NULL)
    {
        mtx_destroy(&resources->counter_mutex);

        station_unload_font_psf2(resources->font);

        if (resources->sdl_window_created)
            station_sdl_destroy_window_context(&resources->sdl_window);

        free(resources);

        return 0;
    }
    else
        return 1;
}

STATION_PLUGIN("Demo plugin", plugin_help, plugin_conf, plugin_init, plugin_final)

