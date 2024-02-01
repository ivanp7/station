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
 * @brief Concurrent processing related functions.
 */

#pragma once
#ifndef _STATION_CONCURRENT_FUN_H_
#define _STATION_CONCURRENT_FUN_H_

#include <station/concurrent.typ.h>

struct station_queue;

/**
 * @brief Initialize concurrent processing context and create threads.
 *
 * busy_wait parameter controls waiting behavior of slave threads.
 *
 * @return 0 if succeed, -1 if arguments are incorrect,
 * 1 if malloc() returned NULL, 2 if thrd_create() returned thrd_nomem,
 * 3 if thrd_create() returned thrd_error.
 */
int
station_concurrent_processing_initialize_context(
        station_concurrent_processing_context_t *context, ///< [out] Context to initialize.
        station_threads_number_t num_threads, ///< [in] Number of concurrent processing threads to create.
        bool busy_wait ///< [in] Whether busy-waiting is enabled.
);

/**
 * @brief Destroy concurrent processing context and join threads.
 */
void
station_concurrent_processing_destroy_context(
        station_concurrent_processing_context_t *context ///< [in] Context to destroy.
);

/**
 * @brief Execute a concurrent processing function.
 *
 * If value of batch_size is zero, it is replaced with ((num_tasks - 1) / context->num_threads) + 1.
 * With this batch size pfunc is called no more than once per thread.
 *
 * If callback function pointer is NULL, the call is blocking
 * does not return until all tasks are done.
 *
 * If callback function pointer is not NULL, the call is non-blocking
 * and returns immediately. When all tasks are done, the callback function
 * is called from one of the threads (which one is unspecified).
 *
 * busy_wait parameter controls waiting behavior of a calling thread if callback is NULL.
 *
 * @return True if threads weren't busy and inputs are correct, otherwise false.
 */
bool
station_concurrent_processing_execute(
        station_concurrent_processing_context_t *context, ///< [in] Concurrent processing context.

        station_tasks_number_t num_tasks,  ///< [in] Number of tasks to be processed.
        station_tasks_number_t batch_size, ///< [in] Number of tasks done by a thread per once.

        station_pfunc_t pfunc, ///< [in] Concurrent processing function.
        void *pfunc_data,      ///< [in] Processed data.

        station_pfunc_callback_t callback, ///< [in] Callback function or NULL.
        void *callback_data,               ///< [in] Callback function data.

        bool busy_wait ///< [in] Whether busy-waiting is enabled.
);

/**
 * @brief Create lock-free queue.
 *
 * Maximum queue capacity is (1 << capacity_log2) elements.
 * Maximum supported value of capacity_log2 is 16 (or 32,
 * if large queues were enabled at configuration time).
 *
 * @return Lock-free queue.
 */
struct station_queue*
station_create_queue(
        size_t element_size,            ///< [in] Queue element size in bytes.
        uint8_t element_alignment_log2, ///< [in] Log2 of queue element alignment in bytes.

        uint8_t capacity_log2 ///< [in] Log2 of maximum capacity of queue.
);

/**
 * @brief Destroy lock-free queue.
 */
void
station_destroy_queue(
        struct station_queue *queue ///< [in] Queue to destroy.
);

/**
 * @brief Push value to lock-free queue.
 *
 * @return True if element was pushed to queue, false if queue is full.
 */
bool
station_queue_push(
        struct station_queue *queue, ///< [in] Queue to push value to.
        const void *value ///< [in] Pointer to pushed value.
);

/**
 * @brief Pop value from lock-free queue.
 *
 * @return True if element was popped from queue, false if queue is empty.
 */
bool
station_queue_pop(
        struct station_queue *queue, ///< [in] Queue to pop value from.
        void *value ///< [out] Memory to write popped value to.
);

/**
 * @brief Get queue capacity.
 *
 * @return Queue capacity.
 */
size_t
station_queue_capacity(
        struct station_queue *queue ///< [in] Queue.
);

/**
 * @brief Get queue element size.
 *
 * @return Queue element size.
 */
size_t
station_queue_element_size(
        struct station_queue *queue ///< [in] Queue.
);

#endif // _STATION_CONCURRENT_FUN_H_

