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
 * @brief Types for parallel processing.
 */

#pragma once
#ifndef _STATION_PARALLEL_TYP_H_
#define _STATION_PARALLEL_TYP_H_

#include <stdint.h>

struct station_parallel_processing_threads_state;

/**
 * @brief Index of a parallel task.
 */
typedef uint32_t station_task_idx_t;
/**
 * @brief Number of parallel tasks.
 */
typedef station_task_idx_t station_tasks_number_t;

/**
 * @brief Index of a thread.
 */
typedef uint16_t station_thread_idx_t;
/**
 * @brief Number of threads.
 */
typedef station_thread_idx_t station_threads_number_t;

/**
 * @brief Parallel processing function.
 */
typedef void (*station_pfunc_t)(
        void *data, ///< [in,out] Processed data.
        station_task_idx_t task_idx,    ///< [in] Index of the current task.
        station_thread_idx_t thread_idx ///< [in] Index of the calling thread.
);

/**
 * @brief Parallel processing callback.
 *
 * This function is called when parallel processing is complete.
 */
typedef void (*station_pfunc_callback_t)(
        void *data, ///< [in,out] Callback data.
        station_thread_idx_t thread_idx ///< [in] Index of the calling thread.
);

/**
 * @brief Parallel processing context.
 */
typedef struct station_parallel_processing_context {
    struct station_parallel_processing_threads_state *state; ///< State of parallel processing threads.
    station_threads_number_t num_threads; ///< Number of parallel processing threads.
} station_parallel_processing_context_t;

#endif // _STATION_PARALLEL_TYP_H_

