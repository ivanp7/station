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

#include <station/signal.fun.h>
#include <station/op.fun.h>
#include <station/fsm.fun.h>

#include <station/state.typ.h>
#include <station/sdl.typ.h>

#include <station/fsm.def.h>

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#  error "threads and/or atomics are not available"
#endif

#include <threads.h>
#include <stdatomic.h>
#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#  include <SDL_video.h>
#  include <SDL_render.h>
#endif

struct station_fsm_context {
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
// signal.fun.h
///////////////////////////////////////////////////////////////////////////////

STATION_SIGNAL_SUPPORT_DEFINITION(SIGHUP)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGINT)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGQUIT)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGUSR1)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGUSR2)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGALRM)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGTERM)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGTSTP)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGTTIN)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGTTOU)
STATION_SIGNAL_SUPPORT_DEFINITION(SIGWINCH)

///////////////////////////////////////////////////////////////////////////////
// op.fun.h
///////////////////////////////////////////////////////////////////////////////

void
station_execute_pfunc(
        station_pfunc_t pfunc,
        void *pfunc_data,

        station_tasks_number_t num_tasks,
        station_tasks_number_t batch_size,

        struct station_fsm_context *context)
{
    assert(context != NULL);

    if ((pfunc == NULL) || (num_tasks == 0))
        return;

    const station_threads_number_t num_threads = context->num_threads;

    if (num_threads > 0)
    {
        if (batch_size == 0)
            batch_size = (num_tasks - 1) / num_threads + 1;

        context->pfunc = pfunc;
        context->pfunc_data = pfunc_data;
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
            pfunc(pfunc_data, task_idx, 0);
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
    struct station_fsm_context *context;
    station_thread_idx_t thread_idx;
};

static
int
station_finite_state_machine_thread(
        void *arg)
{
    struct station_fsm_context *context;
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
        station_state_t state,
        station_threads_number_t num_threads)
{
    if (state.sfunc == NULL)
        return STATION_FSM_EXEC_SUCCESS;

    // Initialize context
    struct station_fsm_context context = {
        .num_threads = num_threads,
    };

    thrd_t *threads = NULL;
    station_thread_idx_t thread_idx = 0;

    uint8_t result = STATION_FSM_EXEC_SUCCESS;

    // Create threads
    if (num_threads > 0)
    {
        threads = malloc(sizeof(*threads) * num_threads);
        if (threads == NULL)
        {
            result = STATION_FSM_EXEC_MALLOC_FAIL;
            goto cleanup;
        }
    }

    for (thread_idx = 0; thread_idx < num_threads; thread_idx++)
    {
        struct station_thread_context *thread_context = malloc(sizeof(*thread_context));
        if (thread_context == NULL)
        {
            result = STATION_FSM_EXEC_MALLOC_FAIL;
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
                result = STATION_FSM_EXEC_THRD_NOMEM;
            else
                result = STATION_FSM_EXEC_THRD_ERROR;

            goto cleanup;
        }

        threads[thread_idx] = thread;
    }

    // Execute finite state machine
    while (state.sfunc != NULL)
        state.sfunc(&state, &context);

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
        station_state_t state,
        station_threads_number_t num_threads,

        const station_sdl_properties_t *sdl_properties,
        station_sdl_context_t *sdl_context)
{
#ifndef STATION_IS_SDL_SUPPORTED
    (void) state;
    (void) num_threads;
    (void) sdl_properties;
    (void) sdl_context;

    return STATION_FSM_EXEC_SDL_NOT_SUPPORTED;
#else
    assert(sdl_properties != NULL);
    assert(sdl_context != NULL);

    if ((sdl_properties->texture_width == 0) || (sdl_properties->texture_height == 0))
        return STATION_FSM_EXEC_INCORRECT_INPUTS;

    // Initialize SDL and resources
    if (SDL_Init(SDL_INIT_VIDEO | sdl_properties->sdl_init_flags) < 0)
        return STATION_FSM_EXEC_SDL_INIT_FAIL;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    uint8_t result = STATION_FSM_EXEC_SUCCESS;

    uint32_t window_flags = 0;
    {
        if (!sdl_properties->window_shown)
            window_flags |= SDL_WINDOW_HIDDEN;

        if (sdl_properties->window_resizable)
            window_flags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(sdl_properties->window_title != NULL ? sdl_properties->window_title : "",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            sdl_properties->window_width != 0 ? sdl_properties->window_width : sdl_properties->texture_width,
            sdl_properties->window_height != 0 ? sdl_properties->window_height : sdl_properties->texture_height,
            window_flags);
    if (window == NULL)
    {
        result = STATION_FSM_EXEC_SDL_CREATE_WINDOW_FAIL;
        goto cleanup;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL)
    {
        result = STATION_FSM_EXEC_SDL_CREATE_RENDERER_FAIL;
        goto cleanup;
    }

    texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            sdl_properties->texture_width, sdl_properties->texture_height);
    if (texture == NULL)
    {
        result = STATION_FSM_EXEC_SDL_CREATE_TEXTURE_FAIL;
        goto cleanup;
    }

    // Initialize SDL context
    sdl_context->window = window;
    sdl_context->renderer = renderer;
    sdl_context->texture = texture;

    sdl_context->texture_lock_rectangle = NULL;
    sdl_context->texture_lock_pixels = NULL;
    sdl_context->texture_lock_pitch = 0;

    sdl_context->texture_width = sdl_properties->texture_width;
    sdl_context->texture_height = sdl_properties->texture_height;

    // Execute finite state machine
    result = station_finite_state_machine(state, num_threads);

    // Clear SDL context
    sdl_context->window = NULL;
    sdl_context->renderer = NULL;
    sdl_context->texture = NULL;

    sdl_context->texture_lock_rectangle = NULL;
    sdl_context->texture_lock_pixels = NULL;
    sdl_context->texture_lock_pitch = 0;

    sdl_context->texture_width = 0;
    sdl_context->texture_height = 0;

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

