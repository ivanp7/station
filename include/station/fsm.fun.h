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

#include <station/state.typ.h>

struct station_sdl_properties;
struct station_sdl_context;

/**
 * @brief Execute a finite state machine.
 *
 * @return Execution status.
 */
uint8_t
station_finite_state_machine(
        station_state_t state, ///< [in] Initial state of finite state machine.
        void *fsm_data, ///< [in,out] Finite state machine data.
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
 * @return Execution status.
 */
uint8_t
station_finite_state_machine_sdl(
        station_state_t state, ///< [in] Initial state of finite state machine.
        void *fsm_data, ///< [in,out] Finite state machine data.
        station_threads_number_t num_threads, ///< [in] Number of parallel processing threads.

        const struct station_sdl_properties *sdl_properties, ///< [in] SDL-related properties of finite state machine.

        struct station_sdl_context *sdl_context ///< [out] SDL context.
);

#endif // _STATION_FSM_FUN_H_

