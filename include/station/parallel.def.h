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
 * @brief Macros for parallel processing.
 */

#pragma once
#ifndef _STATION_PARALLEL_DEF_H_
#define _STATION_PARALLEL_DEF_H_

/**
 * @brief Declarator of a parallel processing function.
 */
#define STATION_PFUNC(name) \
    void name(void *data, station_task_idx_t task_idx, station_thread_idx_t thread_idx)

/**
 * @brief Declarator of a parallel processing callback function.
 */
#define STATION_PFUNC_CALLBACK(name) \
    void name(void *data, station_thread_idx_t thread_idx)

#endif // _STATION_PARALLEL_DEF_H_

