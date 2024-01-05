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

#include <station/state.fun.h>
#include <station/op.fun.h>
#include <station/fsm.fun.h>

#include <station/state.typ.h>
#include <station/sdl.typ.h>
#include <station/index.def.h>

#include <threads.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#  include <SDL_video.h>
#  include <SDL_render.h>
#endif

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#  error "threads and/or atomics are not available"
#endif

struct station_context {
    station_state_t *current_state;

    station_pfunc_t pfunc;
    void *pfunc_data;

    station_tasks_number_t num_tasks;
    station_tasks_number_t batch_size;

    atomic_uint done_tasks;
    atomic_ushort thread_counter;

    atomic_bool ping_flag;
    atomic_bool pong_flag;
    bool ping_sense;
    bool pong_sense;

    bool terminate;

    station_threads_number_t num_threads;
};

///////////////////////////////////////////////////////////////////////////////
// state.fun.h
///////////////////////////////////////////////////////////////////////////////

station_state_t*
station_alloc_state(
        station_states_number_t num_next_states)
{
    return malloc(offsetof(station_state_t, next_state) +
            sizeof(((station_state_t*)NULL)->next_state[0]) * num_next_states);
}

///////////////////////////////////////////////////////////////////////////////
// op.fun.h
///////////////////////////////////////////////////////////////////////////////

void
station_execute_pfunc(
        station_pfunc_t pfunc,
        void *data,

        station_tasks_number_t num_tasks,
        station_tasks_number_t batch_size,

        struct station_context *context)
{
    assert(pfunc != NULL);
    assert(context != NULL);

    if (num_tasks == 0)
        return;

    const station_threads_number_t num_threads = context->num_threads;

    if (num_threads > 0)
    {
        if (batch_size == 0)
            batch_size = (num_tasks - 1) / num_threads + 1;

        context->pfunc = pfunc;
        context->pfunc_data = data;
        context->num_tasks = num_tasks;
        context->batch_size = batch_size;

        atomic_store_explicit(&context->done_tasks, 0, memory_order_relaxed);
        atomic_store_explicit(&context->thread_counter, 0, memory_order_relaxed);

        bool ping_sense = context->ping_sense = !context->ping_sense;
        bool pong_sense = context->pong_sense = !context->pong_sense;

        // Wake slave threads
        atomic_store_explicit(&context->ping_flag, ping_sense, memory_order_relaxed);

        // Wait until all tasks are done
        do
        {
#ifdef STATION_IS_THREAD_YIELD_ENABLED
            thrd_yield();
#endif
        }
        while (atomic_load_explicit(&context->pong_flag, memory_order_relaxed) != pong_sense);
    }
    else
    {
        for (station_task_idx_t task_idx = 0; task_idx < num_tasks; task_idx++)
            pfunc(data, task_idx, 0);
    }
}

uint8_t
station_sdl_lock_texture(
        station_sdl_context_t *sdl_context,
        const struct SDL_Rect *rectangle)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) sdl_context;
    (void) rectangle;
    return -1;
#else
    assert(sdl_context != NULL);
    assert(sdl_context->texture != NULL);
    assert(sdl_context->texture_lock_pixels == NULL);

    void *pixels;
    int pitch;

    if (SDL_LockTexture(sdl_context->texture, rectangle, &pixels, &pitch) < 0)
        return 1;

    sdl_context->texture_lock_rectangle = rectangle;
    sdl_context->texture_lock_pixels = pixels;
    sdl_context->texture_lock_pitch = pitch / sizeof(*sdl_context->texture_lock_pixels);

    return 0;
#endif
}

uint8_t
station_sdl_unlock_texture_and_render(
        station_sdl_context_t *sdl_context)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) sdl_context;
    return -1;
#else
    assert(sdl_context != NULL);
    assert(sdl_context->renderer == NULL);
    assert(sdl_context->texture != NULL);

    SDL_UnlockTexture(sdl_context->texture);

    sdl_context->texture_lock_rectangle = NULL;
    sdl_context->texture_lock_pixels = NULL;
    sdl_context->texture_lock_pitch = 0;

    if (SDL_RenderCopy(sdl_context->renderer, sdl_context->texture,
                (const SDL_Rect*)NULL, (const SDL_Rect*)NULL) < 0)
        return 1;
    SDL_RenderPresent(sdl_context->renderer);

    return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// fsm.fun.h
///////////////////////////////////////////////////////////////////////////////

struct station_thread_context {
    struct station_context *context;
    station_thread_idx_t thread_idx;
};

static
int
station_finite_state_machine_thread(
        void *arg)
{
    struct station_context *context;
    station_thread_idx_t thread_idx;
    {
        assert(arg != NULL);
        struct station_thread_context *thread_context = arg;

        context = thread_context->context;
        thread_idx = thread_context->thread_idx;

        free(thread_context);
    }

    const station_threads_number_t thread_counter_last = context->num_threads - 1;

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
        while (atomic_load_explicit(&context->ping_flag, memory_order_relaxed) != ping_sense);

        if (context->terminate)
            break;

        pfunc = context->pfunc;
        pfunc_data = context->pfunc_data;

        num_tasks = context->num_tasks;
        batch_size = context->batch_size;

        // Acquire first task
        station_task_idx_t task_idx = atomic_fetch_add_explicit(
            &context->done_tasks, batch_size, memory_order_relaxed);
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
                        &context->done_tasks, batch_size, memory_order_relaxed);
                remaining_tasks = batch_size;
            }
        }

        // Wake master thread
        if (atomic_fetch_add_explicit(&context->thread_counter, 1,
                    memory_order_relaxed) == thread_counter_last)
            atomic_store_explicit(&context->pong_flag, pong_sense, memory_order_relaxed);
    }

    return 0;
}

uint8_t
station_finite_state_machine(
        station_state_t *initial_state,
        station_threads_number_t num_threads)
{
    assert(initial_state != NULL);

    uint8_t result = 0;

    // Initialize context
    struct station_context context = {
        .current_state = initial_state,
        .num_threads = num_threads,
    };
    atomic_init(&context.done_tasks, 0);
    atomic_init(&context.thread_counter, 0);
    atomic_init(&context.ping_flag, false);
    atomic_init(&context.pong_flag, false);

    // Create threads
    thrd_t *threads = NULL;
    if (num_threads > 0)
    {
        threads = malloc(sizeof(*threads) * num_threads);
        if (threads == NULL)
            return 1;
    }

    station_thread_idx_t thread_idx;

    for (thread_idx = 0; thread_idx < num_threads; thread_idx++)
    {
        struct station_thread_context *thread_context = malloc(sizeof(*thread_context));
        if (thread_context == NULL)
        {
            result = 1;
            goto cleanup;
        }

        thread_context->context = &context;
        thread_context->thread_idx = thread_idx;

        thrd_t thread;
        int res = thrd_create(&thread, station_finite_state_machine_thread, thread_context);

        if (res != thrd_success)
        {
            free(thread_context);

            if (res == thrd_nomem)
                result = 2;
            else if (res == thrd_error)
                result = 3;
            else
                result = 4;

            goto cleanup;
        }

        threads[thread_idx] = thread;
    }

    // Iterate until there is no next state
    for (;;)
    {
        // Switch to next task
        assert(context.current_state != NULL);
        assert(context.current_state->sfunc != NULL);

        // Execute sequential step
        station_state_idx_t next_state_idx =
            context.current_state->sfunc(context.current_state->state_data, &context);

        if (next_state_idx == STATION_TERMINATOR)
            break;

        context.current_state = context.current_state->next_state[next_state_idx];
    }

cleanup:
    // Wake slave threads and join
    context.terminate = true;
    atomic_store_explicit(&context.ping_flag, !context.ping_sense, memory_order_relaxed);

    for (station_threads_number_t i = 0; i < thread_idx; i++)
        thrd_join(threads[i], (int*)NULL);

    free(threads);

    return result;
}

uint8_t
station_finite_state_machine_sdl(
        struct station_state *initial_state,
        station_threads_number_t num_threads,

        struct station_sdl_context *sdl_context,
        uint32_t sdl_init_flags,

        uint16_t texture_width,
        uint16_t texture_height,

        const char *window_title,
        uint16_t window_width,
        uint16_t window_height)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) initial_state;
    (void) num_threads;
    (void) sdl_context;
    (void) sdl_init_flags;
    (void) texture_width;
    (void) texture_height;
    (void) window_title;
    (void) window_width;
    (void) window_height;
    return -1;
#else
    assert(initial_state != NULL);
    assert(sdl_context != NULL);
    assert(texture_width > 0);
    assert(texture_height > 0);
    assert(window_title != NULL);

    if (window_width == 0)
        window_width = texture_width;
    if (window_height == 0)
        window_height = texture_height;

    uint8_t result = 0;

    // Initialize SDL and resources
    if (SDL_Init(SDL_INIT_VIDEO | sdl_init_flags) < 0)
        return 1;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    window = SDL_CreateWindow(window_title,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            window_width, window_height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        result = 2;
        goto cleanup;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL)
    {
        result = 3;
        goto cleanup;
    }

    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            texture_width, texture_height);
    if (texture == NULL)
    {
        result = 4;
        goto cleanup;
    }

    // Initialize SDL context
    sdl_context->window = window;
    sdl_context->renderer = renderer;
    sdl_context->texture = texture;

    sdl_context->texture_lock_rectangle = NULL;
    sdl_context->texture_lock_pixels = NULL;
    sdl_context->texture_lock_pitch = 0;

    sdl_context->texture_width = texture_width;
    sdl_context->texture_height = texture_height;

    // Execute finite state machine
    result = station_finite_state_machine(initial_state, num_threads);

    if (result != 0)
        result += 4;

cleanup:
    // Release resources and finalize SDL
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();

    return result;
#endif
}

