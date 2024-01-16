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
 * @brief Types for data buffers.
 */

#pragma once
#ifndef _STATION_BUFFER_TYP_H_
#define _STATION_BUFFER_TYP_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Buffer of bytes.
 */
typedef struct station_buffer {
    size_t num_bytes;     ///< Size of data in bytes.
    bool own_memory;      ///< Whether memory is owned by buffer.
    unsigned char *bytes; ///< Data (object representation).
} station_buffer_t;

/**
 * @brief Array of buffers.
 */
typedef struct station_buffers_array {
    size_t num_buffers;
    station_buffer_t **buffers;
} station_buffers_array_t;

#endif // _STATION_BUFFER_TYP_H_

