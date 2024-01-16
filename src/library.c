/*****************************************************************************
 * Copyright (C) 2020-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Station.                                             *
 *                                                                           *
 *   Station is free software: you can redistribute it and/or modify it      *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Station is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Station. If not, see <http://www.gnu.org/licenses/>. *
 *****************************************************************************/

/**
 * @file
 * @brief Library implementation.
 */

#include <station/buffer.fun.h>
#include <station/buffer.typ.h>

#include <station/parallel.fun.h>
#include <station/parallel.typ.h>

#include <station/signal.fun.h>
#include <station/signal.typ.h>

#include <station/sdl.fun.h>
#include <station/sdl.typ.h>

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#  error "threads and/or atomics are not available"
#endif

#include <threads.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
#  include <signal.h>
#  include <pthread.h>
#  include <time.h>
#endif

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#  include <SDL_video.h>
#  include <SDL_render.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// buffer.fun.h
///////////////////////////////////////////////////////////////////////////////

station_buffer_t*
station_create_buffer_from_file(
        const char *file)
{
    FILE *fp = fopen(file, "rb");
    if (fp == NULL)
        return NULL;

    station_buffer_t *buffer = NULL;

    if (fseek(fp, 0, SEEK_END) != 0)
        goto cleanup;

    buffer = malloc(sizeof(*buffer));
    if (buffer == NULL)
        goto cleanup;

    buffer->num_bytes = ftell(fp);
    rewind(fp);

    buffer->own_memory = true;

    buffer->bytes = malloc(buffer->num_bytes);
    if (buffer->bytes == NULL)
        goto cleanup;

    if (fread(buffer->bytes, 1, buffer->num_bytes, fp) != buffer->num_bytes)
        goto cleanup;

    fclose(fp);
    return buffer;

cleanup:
    if (buffer != NULL)
        free(buffer->bytes);
    free(buffer);
    fclose(fp);
    return NULL;
}

void
station_destroy_buffer(
        station_buffer_t *buffer)
{
    if (buffer == NULL)
        return;

    if (buffer->own_memory)
        free(buffer->bytes);

    free(buffer);
}

///////////////////////////////////////////////////////////////////////////////
// parallel.fun.h
///////////////////////////////////////////////////////////////////////////////

struct station_parallel_processing_threads_state {
    struct {
        station_threads_number_t num_threads;
        thrd_t *threads;

        atomic_bool ping_flag;
        atomic_bool pong_flag;
        bool ping_sense;
        bool pong_sense;
    } persistent;

    bool terminate;

    struct {
        station_pfunc_t pfunc;
        void *pfunc_data;

        station_tasks_number_t num_tasks;
        station_tasks_number_t batch_size;

        atomic_uint done_tasks;
        atomic_ushort thread_counter;
    } current;
};

struct station_parallel_processing_thread_arg {
    struct station_parallel_processing_threads_state *threads_state;
    station_thread_idx_t thread_idx;
};

static
int
station_parallel_processing_thread(
        void *arg)
{
    struct station_parallel_processing_threads_state *threads_state;
    station_thread_idx_t thread_idx;
    {
        struct station_parallel_processing_thread_arg *thread_arg = arg;
        assert(thread_arg != NULL);

        threads_state = thread_arg->threads_state;
        thread_idx = thread_arg->thread_idx;

        free(thread_arg);
    }

    const station_threads_number_t thread_counter_last = threads_state->persistent.num_threads - 1;

    station_pfunc_t pfunc;
    void *pfunc_data;

    station_tasks_number_t num_tasks;
    station_tasks_number_t batch_size;

    bool ping_sense = false, pong_sense = false;

    while (true)
    {
        ping_sense = !ping_sense;
        pong_sense = !pong_sense;

        // Wait until signal
        do
        {
#ifdef STATION_IS_THREAD_YIELD_ENABLED
            thrd_yield();
#endif
        }
        while (atomic_load_explicit(&threads_state->persistent.ping_flag,
                    memory_order_relaxed) != ping_sense);

        if (threads_state->terminate)
            break;

        pfunc = threads_state->current.pfunc;
        pfunc_data = threads_state->current.pfunc_data;

        num_tasks = threads_state->current.num_tasks;
        batch_size = threads_state->current.batch_size;

        // Acquire first task
        station_task_idx_t task_idx = atomic_fetch_add_explicit(
                &threads_state->current.done_tasks, batch_size, memory_order_relaxed);
        station_tasks_number_t remaining_tasks = batch_size;

        // Loop until no subtasks left
        while (task_idx < num_tasks)
        {
            // Execute parallel processing function
            pfunc(pfunc_data, task_idx, thread_idx);
            remaining_tasks--;

            // Acquire next task
            if (remaining_tasks > 0)
                task_idx++;
            else
            {
                task_idx = atomic_fetch_add_explicit(
                        &threads_state->current.done_tasks, batch_size, memory_order_relaxed);
                remaining_tasks = batch_size;
            }
        }

        // Wake master thread
        if (atomic_fetch_add_explicit(&threads_state->current.thread_counter, 1,
                    memory_order_relaxed) == thread_counter_last)
            atomic_store_explicit(&threads_state->persistent.pong_flag,
                    pong_sense, memory_order_relaxed);
    }

    return 0;
}

int
station_parallel_processing_initialize_context(
        station_parallel_processing_context_t *context,
        station_threads_number_t num_threads)
{
    if (context == NULL)
        return -1;

    int code;

    // Initialize threads state
    struct station_parallel_processing_threads_state *threads_state = malloc(sizeof(*threads_state));
    if (threads_state == NULL)
        return 1;

    threads_state->persistent.num_threads = num_threads;
    threads_state->persistent.threads = NULL;
    atomic_init(&threads_state->persistent.ping_flag, false);
    atomic_init(&threads_state->persistent.pong_flag, false);
    threads_state->persistent.ping_sense = false;
    threads_state->persistent.pong_sense = false;
    threads_state->terminate = false;

    atomic_init(&threads_state->current.done_tasks, 0);
    atomic_init(&threads_state->current.thread_counter, 0);

    // Create threads
    station_thread_idx_t thread_idx = 0;

    if (num_threads > 0)
    {
        threads_state->persistent.threads = malloc(sizeof(thrd_t) * num_threads);
        if (threads_state->persistent.threads == NULL)
        {
            code = 1;
            goto cleanup;
        }
    }

    for (thread_idx = 0; thread_idx < num_threads; thread_idx++)
    {
        struct station_parallel_processing_thread_arg *thread_arg = malloc(sizeof(*thread_arg));
        if (thread_arg == NULL)
        {
            code = 1;
            goto cleanup;
        }

        thread_arg->threads_state = threads_state;
        thread_arg->thread_idx = thread_idx;

        int res = thrd_create(&threads_state->persistent.threads[thread_idx],
                station_parallel_processing_thread, thread_arg);

        if (res != thrd_success)
        {
            free(thread_arg);

            if (res == thrd_nomem)
                code = 2;
            else
                code = 3;

            goto cleanup;
        }
    }

    context->state = threads_state;
    context->num_threads = num_threads;

    return 0;

cleanup:
    // Wake threads
    threads_state->terminate = true;
    atomic_store_explicit(&threads_state->persistent.ping_flag,
            !threads_state->persistent.ping_sense, memory_order_relaxed);

    for (station_threads_number_t i = 0; i < thread_idx; i++)
        thrd_join(threads_state->persistent.threads[i], (int*)NULL);

    free(threads_state->persistent.threads);
    free(threads_state);

    return code;
}

void
station_parallel_processing_destroy_context(
        station_parallel_processing_context_t *context)
{
    if ((context == NULL) || (context->state == NULL))
        return;

    context->state->terminate = true;
    atomic_store_explicit(&context->state->persistent.ping_flag,
            !context->state->persistent.ping_sense, memory_order_relaxed);

    for (station_threads_number_t i = 0; i < context->state->persistent.num_threads; i++)
        thrd_join(context->state->persistent.threads[i], (int*)NULL);

    free(context->state->persistent.threads);
    free(context->state);

    context->state = NULL;
    context->num_threads = 0;
}

void
station_parallel_processing_execute(
        station_parallel_processing_context_t *context,

        station_pfunc_t pfunc,
        void *pfunc_data,

        station_tasks_number_t num_tasks,
        station_tasks_number_t batch_size)
{
    if ((context == NULL) || (context->state == NULL) ||
            (pfunc == NULL) || (num_tasks == 0))
        return;

    if (context->state->persistent.num_threads > 0)
    {
        if (batch_size == 0)
            batch_size = (num_tasks - 1) / context->state->persistent.num_threads + 1;

        context->state->current.pfunc = pfunc;
        context->state->current.pfunc_data = pfunc_data;
        context->state->current.num_tasks = num_tasks;
        context->state->current.batch_size = batch_size;

        context->state->current.done_tasks = 0;
        context->state->current.thread_counter = 0;

        bool ping_sense = context->state->persistent.ping_sense =
            !context->state->persistent.ping_sense;
        bool pong_sense = context->state->persistent.pong_sense =
            !context->state->persistent.pong_sense;

        // Wake slave threads
        atomic_store_explicit(&context->state->persistent.ping_flag, ping_sense, memory_order_relaxed);

        // Wait until all tasks are done
        do
        {
#ifdef STATION_IS_THREAD_YIELD_ENABLED
            thrd_yield();
#endif
        }
        while (atomic_load_explicit(&context->state->persistent.pong_flag,
                    memory_order_relaxed) != pong_sense);
    }
    else
    {
        for (station_task_idx_t task_idx = 0; task_idx < num_tasks; task_idx++)
            pfunc(pfunc_data, task_idx, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// signal.fun.h
///////////////////////////////////////////////////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED

struct station_signal_management_context
{
    pthread_t thread;
    sigset_t set;

    station_signal_set_t *signals;
    atomic_bool terminate;
};

static
void*
station_signal_management_thread(
        void *arg)
{
    struct station_signal_management_context *context = arg;
    assert(context != NULL);

    struct timespec delay = {.tv_sec = 0, .tv_nsec = 1000000}; // 1 ms

    while (!atomic_load_explicit(&context->terminate, memory_order_acquire))
    {
        int signal = sigtimedwait(&context->set, (siginfo_t*)NULL, &delay);

#define RAISE_SIGNAL(signal)    \
        case signal:            \
            atomic_store_explicit(&context->signals->signal_##signal, true, memory_order_release); \
            break;

        switch (signal)
        {
            RAISE_SIGNAL(SIGHUP)
            RAISE_SIGNAL(SIGINT)
            RAISE_SIGNAL(SIGQUIT)
            RAISE_SIGNAL(SIGUSR1)
            RAISE_SIGNAL(SIGUSR2)
            RAISE_SIGNAL(SIGALRM)
            RAISE_SIGNAL(SIGTERM)
            RAISE_SIGNAL(SIGTSTP)
            RAISE_SIGNAL(SIGTTIN)
            RAISE_SIGNAL(SIGTTOU)
            RAISE_SIGNAL(SIGWINCH)
        }

#undef RAISE_SIGNAL
    }

    return NULL;
}

#endif

struct station_signal_management_context*
station_signal_management_thread_start(
        station_signal_set_t *signals)
{
#ifndef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    (void) signals;

    return NULL;
#else
    if (signals == NULL)
        return NULL;

    struct station_signal_management_context *context = malloc(sizeof(*context));
    if (context == NULL)
        return NULL;

    sigemptyset(&context->set);

#define ADD_SIGNAL(signal)                  \
    if (signals->signal_##signal) {         \
        signals->signal_##signal = false;   \
        sigaddset(&context->set, signal); }

    ADD_SIGNAL(SIGHUP)
    ADD_SIGNAL(SIGINT)
    ADD_SIGNAL(SIGQUIT)
    ADD_SIGNAL(SIGUSR1)
    ADD_SIGNAL(SIGUSR2)
    ADD_SIGNAL(SIGALRM)
    ADD_SIGNAL(SIGTERM)
    ADD_SIGNAL(SIGTSTP)
    ADD_SIGNAL(SIGTTIN)
    ADD_SIGNAL(SIGTTOU)
    ADD_SIGNAL(SIGWINCH)

#undef ADD_SIGNAL

    if (pthread_sigmask(SIG_BLOCK, &context->set, (sigset_t*)NULL) != 0)
        return NULL;

    context->signals = signals;
    atomic_init(&context->terminate, false);

    if (pthread_create(&context->thread, NULL, station_signal_management_thread, context) != 0)
    {
        pthread_sigmask(SIG_UNBLOCK, &context->set, (sigset_t*)NULL);
        free(context);
        return NULL;
    }

    return context;
#endif
}

void
station_signal_management_thread_stop(
        struct station_signal_management_context *context)
{
#ifndef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    (void) context;
#else
    if (context == NULL)
        return;

    atomic_store_explicit(&context->terminate, true, memory_order_release);
    pthread_join(context->thread, (void**)NULL);
    pthread_sigmask(SIG_UNBLOCK, &context->set, (sigset_t*)NULL);
    free(context);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// sdl.fun.h
///////////////////////////////////////////////////////////////////////////////

int
station_sdl_initialize_window_context(
        station_sdl_window_context_t *context,
        const station_sdl_window_properties_t *properties)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;
    (void) properties;

    return -2;
#else
    if ((context == NULL) || (properties == NULL) ||
            (properties->texture.width == 0) || (properties->texture.height == 0))
        return -1;

    int code;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    // Step 1: create window
    window = SDL_CreateWindow(properties->window.title != NULL ? properties->window.title : "",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            properties->window.width != 0 ? properties->window.width : properties->texture.width,
            properties->window.height != 0 ? properties->window.height : properties->texture.height,
            properties->window.flags);

    if (window == NULL)
    {
        code = 1;
        goto cleanup;
    }

    // Step 2: create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (renderer == NULL)
    {
        code = 2;
        goto cleanup;
    }

    // Step 3: create texture
    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            properties->texture.width, properties->texture.height);

    if (texture == NULL)
    {
        code = 3;
        goto cleanup;
    }

    // Set SDL context fields
    context->window.handle = window;

    context->renderer.handle = renderer;

    context->texture.handle = texture;
    context->texture.width = properties->texture.width;
    context->texture.height = properties->texture.height;

    context->texture.lock.rectangle = NULL;
    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;

    return 0;

cleanup:
    if (texture != NULL)
        SDL_DestroyTexture(texture);
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    if (window != NULL)
        SDL_DestroyWindow(window);

    return code;
#endif
}

void
station_sdl_destroy_window_context(
        station_sdl_window_context_t *context)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;
#else
    if (context == NULL)
        return;

    if (context->texture.handle != NULL)
        SDL_DestroyTexture(context->texture.handle);

    if (context->renderer.handle != NULL)
        SDL_DestroyRenderer(context->renderer.handle);

    if (context->window.handle != NULL)
        SDL_DestroyWindow(context->window.handle);

    // Clear SDL context fields
    context->window.handle = NULL;

    context->renderer.handle = NULL;

    context->texture.handle = NULL;
    context->texture.width = 0;
    context->texture.height = 0;

    context->texture.lock.rectangle = NULL;
    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;
#endif
}

int
station_sdl_window_lock_texture(
        station_sdl_window_context_t *context,
        const struct SDL_Rect *rectangle)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;
    (void) rectangle;

    return -2;
#else
    if ((context == NULL) || (context->texture.handle == NULL))
        return -1;

    if (context->texture.lock.pixels != NULL)
        return 2;

    void *pixels;
    int pitch;

    if (SDL_LockTexture(context->texture.handle, rectangle, &pixels, &pitch) < 0)
        return 1;

    context->texture.lock.rectangle = rectangle;
    context->texture.lock.pixels = pixels;
    context->texture.lock.pitch = pitch / sizeof(*context->texture.lock.pixels);

    return 0;
#endif
}

int
station_sdl_window_unlock_texture_and_render(
        station_sdl_window_context_t *context)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;

    return -2;
#else
    if ((context == NULL) || (context->renderer.handle == NULL) || (context->texture.handle == NULL))
        return -1;

    if (context->texture.lock.pixels == NULL)
        return 2;

    SDL_UnlockTexture(context->texture.handle);

    context->texture.lock.rectangle = NULL;
    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;

    if (SDL_RenderCopy(context->renderer.handle, context->texture.handle,
                (const SDL_Rect*)NULL, (const SDL_Rect*)NULL) < 0)
        return 1;

    SDL_RenderPresent(context->renderer.handle);

    return 0;
#endif
}

