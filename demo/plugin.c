#include "plugin.h"

#include <station/op.fun.h>

#include <stdio.h>
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

static STATION_PFUNC(pfunc_draw)
{
    (void) thread_idx;

    struct plugin_resources *resources = data;

    station_task_idx_t y = task_idx / TEXTURE_WIDTH;
    station_task_idx_t x = task_idx % TEXTURE_WIDTH;

    uint32_t pixel = ((x+y) + resources->frame) & 0xFF;
    pixel = (pixel << 16) | (pixel << 8) | pixel;

    resources->sdl_context->texture_lock_pixels[task_idx] = pixel;
}


static STATION_SFUNC(sfunc_pre)
{
    printf("sfunc_pre()\n");

    struct plugin_resources *resources = fsm_data;

    station_execute_pfunc(pfunc_inc, resources, NUM_TASKS, BATCH_SIZE, fsm_context);

    if (resources->counter * 2 != (NUM_TASKS * (NUM_TASKS - 1)))
        printf("counter has incorrect value\n");

    state->sfunc = sfunc_loop;
}

static STATION_SFUNC(sfunc_loop)
{
    (void) fsm_context;

    struct plugin_resources *resources = fsm_data;

#ifdef STATION_IS_SDL_SUPPORTED
    SDL_PollEvent(&resources->event);
#endif

    // Check termination conditions
    if (atomic_load_explicit(&resources->signals->signal_SIGTERM, memory_order_acquire))
    {
        printf("Caught SIGTERM, quitting...\n");
        state->sfunc = sfunc_post;
        return;
    }
    else if (atomic_load_explicit(&resources->signals->signal_SIGINT, memory_order_acquire))
    {
        printf("Caught SIGINT, quitting...\n");
        state->sfunc = sfunc_post;
        return;
    }
#ifdef STATION_IS_SDL_SUPPORTED
    else if (resources->event.type == SDL_QUIT)
    {
        printf("Window is closed, quitting...\n");
        state->sfunc = sfunc_post;
        return;
    }
    else if ((resources->event.type == SDL_KEYDOWN) &&
            (resources->event.key.keysym.sym == SDLK_ESCAPE))
    {
        printf("Escape is pressed, quitting...\n");
        state->sfunc = sfunc_post;
        return;
    }
#endif

    // Process alarm
    if (atomic_load_explicit(&resources->signals->signal_SIGALRM, memory_order_acquire))
    {
        atomic_store_explicit(&resources->signals->signal_SIGALRM, false, memory_order_release);
        printf("ALARM!!!\n");

        if (resources->alarm_set)
            printf("fps = %.1f\n", 1.0f * (resources->frame - resources->prev_frame) / ALARM_DELAY);

        resources->alarm_set = false;
    }

#ifdef STATION_IS_SDL_SUPPORTED
    if ((resources->event.type == SDL_KEYDOWN) &&
            (resources->event.key.keysym.sym == SDLK_SPACE) && !resources->alarm_set)
    {
        printf("Setting an alarm in %d seconds.\n", ALARM_DELAY);
        alarm(ALARM_DELAY);

        resources->alarm_set = true;
        resources->prev_frame = resources->frame;
    }

    // Process freeze
    if (atomic_load_explicit(&resources->signals->signal_SIGTSTP, memory_order_acquire))
    {
        atomic_store_explicit(&resources->signals->signal_SIGTSTP, false, memory_order_release);
        resources->frozen = !resources->frozen;
    }

    // Draw into the window
    if (!resources->frozen)
    {
        if (station_sdl_lock_texture(resources->sdl_context, (const struct SDL_Rect*)NULL) != 0)
        {
            printf("station_sdl_lock_texture() failure\n");
            state->sfunc = sfunc_post;
            return;
        }

        station_execute_pfunc(pfunc_draw, resources,
                TEXTURE_WIDTH*TEXTURE_HEIGHT, BATCH_SIZE, fsm_context);

        if (station_sdl_unlock_texture_and_render(resources->sdl_context) != 0)
        {
            printf("station_sdl_unlock_texture_and_render() failure\n");
            state->sfunc = sfunc_post;
            return;
        }

        resources->frame++;
    }
#endif

    /* thrd_yield(); */
}

static STATION_SFUNC(sfunc_post)
{
    printf("sfunc_post()\n");

    struct plugin_resources *resources = fsm_data;

    station_execute_pfunc(pfunc_dec, resources, NUM_TASKS, BATCH_SIZE, fsm_context);

    if (resources->counter != 0)
        printf("counter has incorrect value\n");

    state->sfunc = NULL;
}


STATION_PLUGIN_HELP(argc, argv)
{
    printf("plugin_help(%i,\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  \"%s\",\n", argv[i]);
    printf(")\n");

    return 0;
}

STATION_PLUGIN_INIT(plugin_resources, initial_state, fsm_data, num_threads, signals, sdl_properties,
        future_sdl_context, future_opencl_context, argc, argv)
{
    (void) num_threads;
    (void) argc;
    (void) argv;

    printf("plugin_init()\n");

    struct plugin_resources *resources = malloc(sizeof(*resources));
    if (resources == NULL)
    {
        printf("malloc() failed\n");
        return 1;
    }

    resources->signals = signals;
    signals->signal_SIGINT = true;
    signals->signal_SIGTERM = true;

    if (sdl_properties != NULL)
    {
        sdl_properties->texture_width = TEXTURE_WIDTH;
        sdl_properties->texture_height = TEXTURE_HEIGHT;

        if (sdl_properties->window_width == 0)
            sdl_properties->window_width = TEXTURE_WIDTH * WINDOW_SCALE;

        if (sdl_properties->window_height == 0)
            sdl_properties->window_height = TEXTURE_HEIGHT * WINDOW_SCALE;

        sdl_properties->window_shown = true;

        if (sdl_properties->window_title == NULL)
            sdl_properties->window_title = "DEMO";
    }

    resources->sdl_context = future_sdl_context;
    resources->opencl_context = future_opencl_context;

    resources->counter = 0;
    mtx_init(&resources->counter_mutex, mtx_plain);

    resources->frozen = false;
    resources->alarm_set = false;
    resources->prev_frame = 0;
    resources->frame = 0;

    *plugin_resources = resources;
    initial_state->sfunc = sfunc_pre;
    *fsm_data = resources;

    return 0;
}

STATION_PLUGIN_FINAL(plugin_resources)
{
    printf("plugin_final()\n");

    struct plugin_resources *resources = plugin_resources;

    if (resources != NULL)
    {
        mtx_destroy(&resources->counter_mutex);
        free(resources);

        return 0;
    }
    else
        return 1;
}

