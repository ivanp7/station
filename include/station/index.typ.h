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
 * @brief Finite state machine index types.
 */

#pragma once
#ifndef _STATION_INDEX_TYP_H_
#define _STATION_INDEX_TYP_H_

#include <stdint.h>

/**
 * @brief FSM state index.
 */
typedef uint8_t station_state_idx_t;
/**
 * @brief Number of FSM states.
 */
typedef station_state_idx_t station_states_number_t;

/**
 * @brief Index of a parallel task.
 */
typedef uint32_t station_task_idx_t;
/**
 * @brief Number of parallel tasks.
 */
typedef station_task_idx_t station_tasks_number_t;

/**
 * @brief Thread index.
 */
typedef uint16_t station_thread_idx_t;
/**
 * @brief Number of threads.
 */
typedef station_thread_idx_t station_threads_number_t;

#endif // _STATION_INDEX_TYP_H_

