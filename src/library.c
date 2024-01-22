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
#include <station/signal.def.h>

#include <station/sdl.fun.h>
#include <station/sdl.typ.h>

#include <station/font.fun.h>
#include <station/font.typ.h>
#include <station/font.def.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#  define STATION_NO_PARALLEL_PROCESSING
#endif

#ifndef STATION_NO_PARALLEL_PROCESSING
#  include <threads.h>
#  include <stdatomic.h>
#endif

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

bool
station_fill_buffer_from_file(
        station_buffer_t* buffer,
        const char *file)
{
    if (buffer == NULL)
        return false;

    *buffer = (station_buffer_t){0};

    if (file == NULL)
        return false;

    FILE *fp = fopen(file, "rb");
    if (fp == NULL)
        return false;

    void *bytes = NULL;

    if (fseek(fp, 0, SEEK_END) != 0)
        goto cleanup;

    size_t num_bytes = ftell(fp);
    rewind(fp);

    bytes = malloc(num_bytes);
    if (bytes == NULL)
        goto cleanup;

    if (fread(bytes, 1, num_bytes, fp) != num_bytes)
        goto cleanup;

    *buffer = (station_buffer_t){
        .num_bytes = num_bytes, .own_memory = true, .bytes = bytes,
    };

    fclose(fp);
    return true;

cleanup:
    free(bytes);
    fclose(fp);
    return false;
}

void
station_clear_buffer(
        station_buffer_t *buffer)
{
    if (buffer == NULL)
        return;

    if (buffer->own_memory)
        free(buffer->bytes);

    *buffer = (station_buffer_t){0};
}

///////////////////////////////////////////////////////////////////////////////
// parallel.fun.h
///////////////////////////////////////////////////////////////////////////////

#ifndef STATION_NO_PARALLEL_PROCESSING

struct station_parallel_processing_threads_state {
    struct {
        station_threads_number_t num_threads;
        thrd_t *threads;

        atomic_flag busy;

        atomic_bool ping_flag;
        atomic_bool pong_flag;
        bool ping_sense;
        bool pong_sense;

#ifdef STATION_IS_CONDITION_VARIABLES_USED
        cnd_t ping_cnd;
        cnd_t pong_cnd;
        mtx_t ping_mtx;
        mtx_t pong_mtx;
#endif
    } persistent;

    bool terminate;

    struct {
        station_pfunc_t pfunc;
        void *pfunc_data;

        station_pfunc_callback_t callback;
        void *callback_data;

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

    station_pfunc_callback_t callback;
    void *callback_data;

    station_tasks_number_t num_tasks;
    station_tasks_number_t batch_size;

    bool ping_sense = false, pong_sense = false;

    for (;;)
    {
        ping_sense = !ping_sense;
        pong_sense = !pong_sense;

#ifdef STATION_IS_CONDITION_VARIABLES_USED
        {
#  ifndef NDEBUG
            int res =
#  endif
                mtx_lock(&threads_state->persistent.ping_mtx);
            assert(res == thrd_success);
        }
#endif

        // Wait until signal
        while (atomic_load_explicit(&threads_state->persistent.ping_flag,
                    memory_order_acquire) != ping_sense)
        {
#ifdef STATION_IS_CONDITION_VARIABLES_USED
#  ifndef NDEBUG
            int res =
#  endif
                cnd_wait(&threads_state->persistent.ping_cnd,
                        &threads_state->persistent.ping_mtx);
            assert(res == thrd_success);
#endif
        }

#ifdef STATION_IS_CONDITION_VARIABLES_USED
        {
#  ifndef NDEBUG
            int res =
#  endif
                mtx_unlock(&threads_state->persistent.ping_mtx);
            assert(res == thrd_success);
        }
#endif

        if (threads_state->terminate)
            break;

        pfunc = threads_state->current.pfunc;
        pfunc_data = threads_state->current.pfunc_data;

        callback = threads_state->current.callback;
        callback_data = threads_state->current.callback_data;

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

        // Check if the current thread is the last
        if (atomic_fetch_add_explicit(&threads_state->current.thread_counter, 1,
                    memory_order_seq_cst) == thread_counter_last)
        {
            atomic_store_explicit(&threads_state->persistent.pong_flag,
                    pong_sense, memory_order_release);

            if (callback != NULL)
                callback(callback_data, thread_idx);
            else
            {
                // Wake master thread
#ifdef STATION_IS_CONDITION_VARIABLES_USED
                {
#  ifndef NDEBUG
                    int res =
#  endif
                        mtx_lock(&threads_state->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
                cnd_broadcast(&threads_state->persistent.pong_cnd);
                {
#  ifndef NDEBUG
                    int res =
#  endif
                        mtx_unlock(&threads_state->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
#endif
            }

            // Threads are no longer busy
            atomic_flag_clear_explicit(&threads_state->persistent.busy, memory_order_release);
        }
    }

    return 0;
}

#endif // STATION_NO_PARALLEL_PROCESSING

int
station_parallel_processing_initialize_context(
        station_parallel_processing_context_t *context,
        station_threads_number_t num_threads)
{
#ifdef STATION_NO_PARALLEL_PROCESSING
    (void) context;

    if (num_threads > 0)
        return -2;

    context->state = NULL;
    context->num_threads = 0;

    return 0;
#else
    if (context == NULL)
        return -1;

    int code;

    // Initialize threads state
    struct station_parallel_processing_threads_state *threads_state = malloc(sizeof(*threads_state));
    if (threads_state == NULL)
        return 1;

    threads_state->persistent.num_threads = num_threads;
    threads_state->persistent.threads = NULL;

    threads_state->persistent.busy = (atomic_flag)ATOMIC_FLAG_INIT;

    atomic_init(&threads_state->persistent.ping_flag, false);
    atomic_init(&threads_state->persistent.pong_flag, false);
    threads_state->persistent.ping_sense = false;
    threads_state->persistent.pong_sense = false;

#ifdef STATION_IS_CONDITION_VARIABLES_USED
    {
        int res;

        res = cnd_init(&threads_state->persistent.ping_cnd);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                return 2;
            else
                return 3;
        }

        res = cnd_init(&threads_state->persistent.pong_cnd);
        if (res != thrd_success)
        {
            cnd_destroy(&threads_state->persistent.ping_cnd);

            if (res == thrd_nomem)
                return 2;
            else
                return 3;
        }

        res = mtx_init(&threads_state->persistent.ping_mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&threads_state->persistent.ping_cnd);
            cnd_destroy(&threads_state->persistent.pong_cnd);

            return 3;
        }

        res = mtx_init(&threads_state->persistent.pong_mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&threads_state->persistent.ping_cnd);
            cnd_destroy(&threads_state->persistent.pong_cnd);
            mtx_destroy(&threads_state->persistent.ping_mtx);

            return 3;
        }
    }
#endif

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
        struct station_parallel_processing_thread_arg *thread_arg =
            malloc(sizeof(*thread_arg));
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
            !threads_state->persistent.ping_sense, memory_order_release);

#ifdef STATION_IS_CONDITION_VARIABLES_USED
    {
#  ifndef NDEBUG
        int res =
#  endif
            mtx_lock(&threads_state->persistent.ping_mtx);
        assert(res == thrd_success);
    }
    cnd_broadcast(&threads_state->persistent.ping_cnd);
    {
#  ifndef NDEBUG
        int res =
#  endif
            mtx_unlock(&threads_state->persistent.ping_mtx);
        assert(res == thrd_success);
    }
#endif

    for (station_threads_number_t i = 0; i < thread_idx; i++)
        thrd_join(threads_state->persistent.threads[i], (int*)NULL);

    free(threads_state->persistent.threads);

#ifdef STATION_IS_CONDITION_VARIABLES_USED
    cnd_destroy(&threads_state->persistent.ping_cnd);
    cnd_destroy(&threads_state->persistent.pong_cnd);
    mtx_destroy(&threads_state->persistent.ping_mtx);
    mtx_destroy(&threads_state->persistent.pong_mtx);
#endif

    free(threads_state);

    return code;
#endif // STATION_NO_PARALLEL_PROCESSING
}

void
station_parallel_processing_destroy_context(
        station_parallel_processing_context_t *context)
{
#ifdef STATION_NO_PARALLEL_PROCESSING
    (void) context;

    return;
#else
    if ((context == NULL) || (context->state == NULL))
        return;

    context->state->terminate = true;

    atomic_store_explicit(&context->state->persistent.ping_flag,
            !context->state->persistent.ping_sense, memory_order_release);
#ifdef STATION_IS_CONDITION_VARIABLES_USED
    {
#  ifndef NDEBUG
        int res =
#  endif
            mtx_lock(&context->state->persistent.ping_mtx);
        assert(res == thrd_success);
    }
    cnd_broadcast(&context->state->persistent.ping_cnd);
    {
#  ifndef NDEBUG
        int res =
#  endif
            mtx_unlock(&context->state->persistent.ping_mtx);
        assert(res == thrd_success);
    }
#endif

    for (station_threads_number_t i = 0; i < context->state->persistent.num_threads; i++)
        thrd_join(context->state->persistent.threads[i], (int*)NULL);

    free(context->state->persistent.threads);

#ifdef STATION_IS_CONDITION_VARIABLES_USED
    cnd_destroy(&context->state->persistent.ping_cnd);
    cnd_destroy(&context->state->persistent.pong_cnd);
    mtx_destroy(&context->state->persistent.ping_mtx);
    mtx_destroy(&context->state->persistent.pong_mtx);
#endif

    free(context->state);

    context->state = NULL;
    context->num_threads = 0;
#endif // STATION_NO_PARALLEL_PROCESSING
}

bool
station_parallel_processing_execute(
        station_parallel_processing_context_t *context,

        station_tasks_number_t num_tasks,
        station_tasks_number_t batch_size,

        station_pfunc_t pfunc,
        void *pfunc_data,

        station_pfunc_callback_t callback,
        void *callback_data)
{
#ifdef STATION_NO_PARALLEL_PROCESSING
    (void) context;
    (void) batch_size;

    if (pfunc == NULL)
        return false;

    for (station_task_idx_t task_idx = 0; task_idx < num_tasks; task_idx++)
        pfunc(pfunc_data, task_idx, 0);

    if (callback != NULL)
        callback(callback_data, 0);

    return true;
#else
    if ((context == NULL) || (context->state == NULL) ||
            (pfunc == NULL) || (num_tasks == 0))
        return false;

    if (context->state->persistent.num_threads > 0)
    {
        // Check if threads are busy, and set the flag if not
        if (atomic_flag_test_and_set_explicit(&context->state->persistent.busy,
                    memory_order_acquire))
            return false;

        if (batch_size == 0) // automatic batch size
            batch_size = (num_tasks - 1) / context->state->persistent.num_threads + 1;

        context->state->current.pfunc = pfunc;
        context->state->current.pfunc_data = pfunc_data;

        context->state->current.callback = callback;
        context->state->current.callback_data = callback_data;

        context->state->current.num_tasks = num_tasks;
        context->state->current.batch_size = batch_size;

        context->state->current.done_tasks = 0;
        context->state->current.thread_counter = 0;

        bool ping_sense = context->state->persistent.ping_sense =
            !context->state->persistent.ping_sense;
        bool pong_sense = context->state->persistent.pong_sense =
            !context->state->persistent.pong_sense;

        // Wake slave threads
        atomic_store_explicit(&context->state->persistent.ping_flag, ping_sense, memory_order_release);

#ifdef STATION_IS_CONDITION_VARIABLES_USED
        {
#  ifndef NDEBUG
            int res =
#  endif
                mtx_lock(&context->state->persistent.ping_mtx);
            assert(res == thrd_success);
        }
        cnd_broadcast(&context->state->persistent.ping_cnd);
        {
#  ifndef NDEBUG
            int res =
#  endif
                mtx_unlock(&context->state->persistent.ping_mtx);
            assert(res == thrd_success);
        }
#endif

#ifdef STATION_IS_CONDITION_VARIABLES_USED
        {
#  ifndef NDEBUG
            int res =
#  endif
                mtx_lock(&context->state->persistent.pong_mtx);
            assert(res == thrd_success);
        }
#endif

        if (callback == NULL)
        {
            // Wait until all tasks are done
            while (atomic_load_explicit(&context->state->persistent.pong_flag,
                        memory_order_acquire) != pong_sense)
            {
#ifdef STATION_IS_CONDITION_VARIABLES_USED
#  ifndef NDEBUG
                int res =
#  endif
                    cnd_wait(&context->state->persistent.pong_cnd,
                            &context->state->persistent.pong_mtx);
                assert(res == thrd_success);
#endif
            }

#ifdef STATION_IS_CONDITION_VARIABLES_USED
            {
#  ifndef NDEBUG
                int res =
#  endif
                    mtx_unlock(&context->state->persistent.pong_mtx);
                assert(res == thrd_success);
            }
#endif
        }
    }
    else
    {
        for (station_task_idx_t task_idx = 0; task_idx < num_tasks; task_idx++)
            pfunc(pfunc_data, task_idx, 0);

        if (callback != NULL)
            callback(callback_data, 0);
    }

    return true;
#endif // STATION_NO_PARALLEL_PROCESSING
}

///////////////////////////////////////////////////////////////////////////////
// signal.fun.h
///////////////////////////////////////////////////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED

#define SIGTIMEDWAIT_TIMEOUT_NANO 1000000 // 1 ms

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

    struct timespec delay = {.tv_sec = 0, .tv_nsec = SIGTIMEDWAIT_TIMEOUT_NANO};

    while (!atomic_load_explicit(&context->terminate, memory_order_acquire))
    {
        int signal = sigtimedwait(&context->set, (siginfo_t*)NULL, &delay);

#define RAISE_SIGNAL(signal)    \
        case signal:            \
            STATION_SIGNAL_SET_FLAG(&context->signals->signal_##signal); \
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

    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;
    context->texture.lock.rectangle.x = 0;
    context->texture.lock.rectangle.y = 0;
    context->texture.lock.rectangle.width = 0;
    context->texture.lock.rectangle.height = 0;

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

    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;
    context->texture.lock.rectangle.x = 0;
    context->texture.lock.rectangle.y = 0;
    context->texture.lock.rectangle.width = 0;
    context->texture.lock.rectangle.height = 0;
#endif
}

int
station_sdl_window_lock_texture(
        station_sdl_window_context_t *context,

        bool whole_texture,

        uint32_t x,
        uint32_t y,
        uint32_t width,
        uint32_t height)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;
    (void) whole_texture;
    (void) x;
    (void) y;
    (void) width;
    (void) height;

    return -2;
#else
    if ((context == NULL) || (context->texture.handle == NULL))
        return -1;

    if (!whole_texture && ((width == 0) || (height == 0)))
        return -1;

    if (context->texture.lock.pixels != NULL)
        return 2;

    struct SDL_Rect rectangle = {.x = x, .y = y, .w = width, .h = height};

    void *pixels;
    int pitch;

    if (SDL_LockTexture(context->texture.handle,
                whole_texture ? NULL : &rectangle, &pixels, &pitch) < 0)
        return 1;

    context->texture.lock.pixels = pixels;
    context->texture.lock.pitch = pitch / sizeof(*context->texture.lock.pixels);

    if (whole_texture)
    {
        context->texture.lock.rectangle.x = 0;
        context->texture.lock.rectangle.y = 0;
        context->texture.lock.rectangle.width = context->texture.width;
        context->texture.lock.rectangle.height = context->texture.height;
    }
    else
    {
        context->texture.lock.rectangle.x = x;
        context->texture.lock.rectangle.y = y;
        context->texture.lock.rectangle.width = width;
        context->texture.lock.rectangle.height = height;
    }

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

    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;
    context->texture.lock.rectangle.x = 0;
    context->texture.lock.rectangle.y = 0;
    context->texture.lock.rectangle.width = 0;
    context->texture.lock.rectangle.height = 0;

    if (SDL_RenderCopy(context->renderer.handle, context->texture.handle,
                (const SDL_Rect*)NULL, (const SDL_Rect*)NULL) < 0)
        return 1;

    SDL_RenderPresent(context->renderer.handle);

    return 0;
#endif
}

bool
station_sdl_window_texture_draw_glyph(
        station_sdl_window_context_t *context,

        int32_t x,
        int32_t y,

        bool draw_fg,
        bool draw_bg,

        uint32_t fg,
        uint32_t bg,

        const unsigned char *glyph,
        uint32_t glyph_width,
        uint32_t glyph_height,

        int32_t glyph_col_idx,
        int32_t glyph_row_idx,
        int32_t glyph_num_cols,
        int32_t glyph_num_rows)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) context;
    (void) x;
    (void) y;
    (void) draw_fg;
    (void) draw_bg;
    (void) fg;
    (void) bg;
    (void) glyph;
    (void) glyph_width;
    (void) glyph_height;
    (void) glyph_col_idx;
    (void) glyph_row_idx;
    (void) glyph_num_cols;
    (void) glyph_num_rows;

    return false;
#else
    if ((context == NULL) || (context->texture.lock.pixels == NULL))
        return false;
    else if (!draw_fg && !draw_bg)
        return false;
    else if ((glyph == NULL) || (glyph_width == 0) || (glyph_height == 0))
        return false;
    else if ((glyph_num_cols == 0) || (glyph_num_rows == 0))
        return false;

    uint32_t *pixels = context->texture.lock.pixels;
    uint32_t pitch = context->texture.lock.pitch;
    uint32_t rect_x = context->texture.lock.rectangle.x;
    uint32_t rect_y = context->texture.lock.rectangle.y;
    uint32_t rect_width = context->texture.lock.rectangle.width;
    uint32_t rect_height = context->texture.lock.rectangle.height;

    uint32_t bytes_per_row = (glyph_width + 7) / 8;

    int32_t col_idx_delta = glyph_num_cols > 0 ? 1 : -1;
    int32_t row_idx_delta = glyph_num_rows > 0 ? 1 : -1;

    for (int32_t row_idx = glyph_row_idx, i = 0; row_idx != glyph_row_idx + glyph_num_rows;
            row_idx += row_idx_delta, i++)
    {
        if ((y + i < 0) || ((uint32_t)(y + i) < rect_y) || ((uint32_t)(y + i) >= rect_y + rect_height))
            continue;

        uint32_t texture_row_displ = pitch * (y + i - rect_y);

        const unsigned char *row;
        if ((row_idx >= 0) && ((uint32_t)row_idx < glyph_height))
            row = glyph + bytes_per_row * row_idx;
        else
            row = NULL;

        uint32_t byte_idx = bytes_per_row;
        unsigned char byte = 0;

        for (int32_t col_idx = glyph_col_idx, j = 0; col_idx != glyph_col_idx + glyph_num_cols;
                col_idx += col_idx_delta, j++)
        {
            if ((x + j < 0) || ((uint32_t)(x + j) < rect_x) || ((uint32_t)(x + j) >= rect_x + rect_width))
                continue;

            uint32_t texture_idx = texture_row_displ + (x + j - rect_x);

            bool pixel_is_fg = false;
            if ((row != NULL) && (col_idx >= 0) && ((uint32_t)col_idx < glyph_width))
            {
                uint32_t current_byte_idx = col_idx / 8;
                if (current_byte_idx != byte_idx)
                {
                    byte_idx = current_byte_idx;
                    byte = row[byte_idx];
                }

                pixel_is_fg = byte & (1 << (7 - (col_idx % 8)));
            }

            if (draw_fg && pixel_is_fg)
                pixels[texture_idx] = fg;
            else if (draw_bg && !pixel_is_fg)
                pixels[texture_idx] = bg;
        }
    }

    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// font.fun.h
///////////////////////////////////////////////////////////////////////////////

#define NUM_UNICODE_CODE_POINTS 0x110000 // 0 - 0x10FFFF

static
size_t
station_decode_utf8_code_point(
        const unsigned char *seq,
        size_t remaining_bytes,
        uint32_t *code_point)
{
    *code_point = -1; // invalid code point signifies error

    if (remaining_bytes == 0)
        return 0;

    unsigned char byte1 = seq[0];

    if ((byte1 & 0xF8) == 0xF8) // invalid first byte
    {
        if (byte1 == 0xFF) // record separator byte
            *code_point = NUM_UNICODE_CODE_POINTS;
        return 1;
    }

    if (byte1 & 0x80) // Unicode
    {
        if (byte1 & 0x40) // valid first byte
        {
            if (remaining_bytes < 2)
                return 1;

            unsigned char byte2 = seq[1];
            if ((byte2 & 0xC0) != 0x80)
                return 1;

            if (byte1 & 0x20) // 3-4 bytes
            {
                if (remaining_bytes < 3)
                    return 2;

                unsigned char byte3 = seq[2];
                if ((byte3 & 0xC0) != 0x80)
                    return 2;

                if (byte1 & 0x10) // 4 bytes
                {
                    if (remaining_bytes < 4)
                        return 3;

                    unsigned char byte4 = seq[3];
                    if ((byte4 & 0xC0) != 0x80)
                        return 3;

                    *code_point = byte1 & 0x07;
                    *code_point = (*code_point << 6) | (byte2 & 0x3F);
                    *code_point = (*code_point << 6) | (byte3 & 0x3F);
                    *code_point = (*code_point << 6) | (byte4 & 0x3F);
                    return 4;
                }
                else // 3 bytes
                {
                    *code_point = byte1 & 0x0F;
                    *code_point = (*code_point << 6) | (byte2 & 0x3F);
                    *code_point = (*code_point << 6) | (byte3 & 0x3F);
                    return 3;
                }
            }
            else // 2 bytes
            {
                *code_point = byte1 & 0x1F;
                *code_point = (*code_point << 6) | (byte2 & 0x3F);
                return 2;
            }
        }
        else // invalid first byte
        {
            // Skip until valid first byte
            size_t skip = 1;
            while ((skip < remaining_bytes) && ((seq[skip] & 0xC0) == 0x80))
                skip++;

            return skip;
        }
    }
    else // ASCII
    {
        *code_point = byte1;
        return 1;
    }
}

station_font_psf2_t*
station_load_font_psf2_from_buffer(
        const station_buffer_t *buffer)
{
    if ((buffer == NULL) || (buffer->bytes == NULL))
        return NULL;

    if (buffer->num_bytes < sizeof(station_font_psf2_header_t))
        return NULL;

    station_font_psf2_header_t *header = buffer->bytes;
    if ((header->magic != STATION_FONT_PSF2_MAGIC) || (header->version != 0))
        return NULL;

    if (header->header_size < sizeof(station_font_psf2_header_t) ||
            (header->bytes_per_glyph == 0) || (header->num_glyphs == 0))
        return NULL;

    if (buffer->num_bytes < header->header_size +
            (size_t)header->bytes_per_glyph * header->num_glyphs)
        return NULL;

    station_font_psf2_t *font = malloc(sizeof(*font));
    if (font == NULL)
        return NULL;

    font->header = header;
    font->glyphs = (unsigned char*)buffer->bytes + header->header_size;

    if (!header->flags)
        font->mapping_table = NULL;
    else
    {
        unsigned char *table_end = (unsigned char*)buffer->bytes + buffer->num_bytes;
        unsigned char *table = font->glyphs +
            (size_t)header->bytes_per_glyph * header->num_glyphs;

        size_t remaining_bytes = table_end - table;

        font->mapping_table = malloc(sizeof(*font->mapping_table) * NUM_UNICODE_CODE_POINTS);
        if (font->mapping_table == NULL)
        {
            free(font);
            return NULL;
        }

        for (uint32_t i = 0; i < NUM_UNICODE_CODE_POINTS; i++)
            font->mapping_table[i] = 0; // map all code points to glyph #0 by default

        // Decode mapping table
        uint32_t glyph_idx = 0;
        while (remaining_bytes > 0)
        {
            uint32_t code_point;
            size_t seq_len = station_decode_utf8_code_point(table, remaining_bytes, &code_point);

            if (code_point < NUM_UNICODE_CODE_POINTS) // valid code point
                font->mapping_table[code_point] = glyph_idx;
            else if (code_point == NUM_UNICODE_CODE_POINTS) // record end
                glyph_idx++;

            table += seq_len;
            remaining_bytes -= seq_len;
        }
    }

    return font;
}

void
station_unload_font_psf2(
        station_font_psf2_t *font)
{
    if (font != NULL)
    {
        free(font->mapping_table);
        free(font);
    }
}

const unsigned char*
station_font_psf2_glyph(
        const char *utf8_str,
        size_t utf8_str_len,

        size_t *chr_len,
        const station_font_psf2_t *font)
{
    if ((utf8_str == NULL) || (font == NULL))
        return NULL;

    uint32_t code_point;
    size_t seq_len = station_decode_utf8_code_point(
            (const unsigned char*)utf8_str, utf8_str_len, &code_point);

    if (chr_len != NULL)
        *chr_len = seq_len;

    if (code_point < NUM_UNICODE_CODE_POINTS) // valid code point
    {
        uint32_t glyph_idx = (font->mapping_table == NULL) ?
            code_point : font->mapping_table[code_point];

        if (glyph_idx >= font->header->num_glyphs)
            return NULL;

        return font->glyphs + (size_t)font->header->bytes_per_glyph * glyph_idx;
    }
    else
        return NULL;
}

