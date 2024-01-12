#include <station/plugin.h>
#include <station/func.def.h>
#include <station/op.fun.h>
#include <station/signal.fun.h>
#include <station/signal.typ.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <threads.h>
#include <unistd.h>

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#endif

#define NUM_TASKS 128
#define BATCH_SIZE 16

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 144
#define WINDOW_SCALE 4

#define ALARM_DELAY 3

struct plugin_resources {
    station_signal_set_t *signals;

#ifdef STATION_IS_SDL_SUPPORTED
    SDL_Event event;
#endif
    station_sdl_context_t *sdl_context;
    station_opencl_context_t *opencl_context;

    int counter;
    mtx_t counter_mutex;

    bool frozen;
    bool alarm_set;
    unsigned frame;
};

static STATION_PFUNC(pfunc_inc);
static STATION_PFUNC(pfunc_dec);

static STATION_SFUNC(sfunc_pre);
static STATION_SFUNC(sfunc_loop);
static STATION_SFUNC(sfunc_post);

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
        resources->alarm_set = false;
        printf("ALARM!!!\n");
    }

#ifdef STATION_IS_SDL_SUPPORTED
    if ((resources->event.type == SDL_KEYDOWN) &&
            (resources->event.key.keysym.sym == SDLK_SPACE) && !resources->alarm_set)
    {
        printf("Setting an alarm in %d seconds.\n", ALARM_DELAY);
        alarm(ALARM_DELAY);
        resources->alarm_set = true;
    }

    // Process freeze
    if (atomic_load_explicit(&resources->signals->signal_SIGTSTP, memory_order_acquire))
    {
        atomic_store_explicit(&resources->signals->signal_SIGTSTP, false, memory_order_release);
        resources->frozen = !resources->frozen;
    }

    // Draw into the window
    if (station_sdl_lock_texture(resources->sdl_context, (const struct SDL_Rect*)NULL) != 0)
    {
        printf("station_sdl_lock_texture() failure\n");
        state->sfunc = sfunc_post;
        return;
    }

    for (unsigned y = 0; y < TEXTURE_HEIGHT; y++)
        for (unsigned x = 0; x < TEXTURE_WIDTH; x++)
        {
            uint32_t pixel = ((x+y) + resources->frame) & 0xFF;

            pixel = (pixel << 16) | (pixel << 8) | pixel;

            resources->sdl_context->texture_lock_pixels[
                y * resources->sdl_context->texture_lock_pitch + x] = pixel;
        }

    if (!resources->frozen)
        resources->frame++;

    if (station_sdl_unlock_texture_and_render(resources->sdl_context) != 0)
    {
        printf("station_sdl_unlock_texture_and_render() failure\n");
        state->sfunc = sfunc_post;
        return;
    }
#endif

    thrd_yield();
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

STATION_PLUGIN_PREAMBLE()

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

