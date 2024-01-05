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
 * @brief Finite state machine execution.
 */

#pragma once
#ifndef _STATION_FSM_FUN_H_
#define _STATION_FSM_FUN_H_

#include <station/index.typ.h>

struct station_state;
struct station_sdl_context;

/**
 * @brief Execute a finite state machine.
 *
 * @return 0 on success, 1 on malloc() failure, 2-4 on thrd_create() failure
 * (2: thrd_nomem, 3: thrd_error, 4: other).
 */
uint8_t
station_finite_state_machine(
        struct station_state *initial_state, ///< [in] Initial state of finite state machine.
        station_threads_number_t num_threads ///< [in] Number of parallel processing threads.
);

/**
 * @brief Execute a finite state machine with SDL support.
 *
 * This function initializes SDL library, creates SDL context,
 * and then executes a finite state machine.
 *
 * If either window width or height is zero, it is substituted with the image width or height.
 *
 * Initialization steps:
 * 1. SDL_Init()
 * 2. SDL_CreateWindow()
 * 3. SDL_CreateRenderer()
 * 4. SDL_CreateTexture()
 *
 * @return Zero on success, or number of failed initialization step.
 * Higher return codes correspond to non-zero return codes of station_finite_state_machine() plus 4.
 * In case SDL is not supported, this function returns maximum value of the return type.
 */
uint8_t
station_finite_state_machine_sdl(
        struct station_state *initial_state,  ///< [in] Initial state of finite state machine.
        station_threads_number_t num_threads, ///< [in] Number of parallel processing threads.

        struct station_sdl_context *sdl_context, ///< [out] SDL context.
        uint32_t sdl_init_flags, ///< [in] Flags to pass to SDL_Init() call.

        uint16_t texture_width,  ///< [in] Texture width in pixels.
        uint16_t texture_height, ///< [in] Texture height in pixels.

        const char *window_title, ///< [in] Window title.
        uint16_t window_width,    ///< [in] Window width in pixels.
        uint16_t window_height    ///< [in] Window height in pixels.
);

#endif // _STATION_FSM_FUN_H_

