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
 * @brief Finite state machine function types.
 */

#pragma once
#ifndef _STATION_FUNC_TYP_H_
#define _STATION_FUNC_TYP_H_

#include <station/index.typ.h>

struct station_state;
struct station_fsm_context;

/**
 * @brief State function of a finite state machine.
 *
 * State function is supposed to modify 'state' argument, setting it to the next FSM state.
 * If the next state function is NULL, then FSM execution is terminated.
 */
typedef void (*station_sfunc_t)(
        struct station_state *state, ///< [in,out] Current state.
        struct station_fsm_context *fsm_context ///< [in] Finite state machine context.
);

/**
 * @brief Parallel processing function of a finite state machine.
 */
typedef void (*station_pfunc_t)(
        void *data, ///< [in,out] Processed data.
        station_task_idx_t task_idx,    ///< [in] Index of the current task.
        station_thread_idx_t thread_idx ///< [in] Index of the current thread.
);

#endif // _STATION_FUNC_TYP_H_

