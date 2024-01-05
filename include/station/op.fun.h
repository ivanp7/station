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
 * @brief Operations in state functions.
 */

#pragma once
#ifndef _STATION_OP_FUN_H_
#define _STATION_OP_FUN_H_

#include <station/func.typ.h>

struct station_context;
struct station_sdl_context;
struct SDL_Rect;

/**
 * @brief Execute a parallel processing function from inside a state function.
 *
 * Does not return until all tasks are done.
 *
 * If value of batch_size is zero, it is replaced with ((num_tasks - 1) / num_threads) + 1.
 * This batch size leads to pfunc being called no more than once per thread.
 */
void
station_execute_pfunc(
        station_pfunc_t pfunc, ///< [in] Parallel processing function.
        void *data,   ///< [in] Processed data.

        station_tasks_number_t num_tasks,  ///< [in] Number of tasks to be processed.
        station_tasks_number_t batch_size, ///< [in] Number of tasks done by a thread per once.

        struct station_context *context ///< [in] Finite state machine context.
);

/**
 * @brief Lock SDL texture for writing.
 *
 * @return Zero on success, non-zero on failure.
 * In case SDL is not supported, this function returns maximum value of the return type.
 */
uint8_t
station_sdl_lock_texture(
        struct station_sdl_context *sdl_context, ///< [in,out] SDL context.
        const struct SDL_Rect *rectangle ///< [in] Rectangle to lock, or NULL for full texture.
);

/**
 * @brief Unlock SDL texture and render updated content to the window.
 *
 * @return Zero on success, non-zero on failure.
 * In case SDL is not supported, this function returns maximum value of the return type.
 */
uint8_t
station_sdl_unlock_texture_and_render(
        struct station_sdl_context *sdl_context ///< [in,out] SDL context.
);

#endif // _STATION_OP_FUN_H_

