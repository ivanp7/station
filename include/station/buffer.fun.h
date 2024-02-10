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
 * @brief Operations on data buffers.
 */

#pragma once
#ifndef _STATION_BUFFER_FUN_H_
#define _STATION_BUFFER_FUN_H_

#include <stdio.h>
#include <stdbool.h>

struct station_buffer;

/**
 * @brief Fill buffer with file contents.
 *
 * Buffer structure fields are discarded and overwritten.
 * In case of failure, buffer fields are set to default values.
 *
 * @return Operation success status (true -- success, false -- failure).
 */
bool
station_fill_buffer_from_file(
        struct station_buffer *buffer, ///< [in] Buffer to read file contents into.
        FILE *file ///< [in] Binary stream to read buffer contents from.
);

/**
 * @brief Resize buffer.
 *
 * Buffer can only be resized if its memory is owned.
 *
 * @return Operation success status (true -- success, false -- failure).
 */
bool
station_resize_buffer(
        struct station_buffer *buffer, ///< [in] Buffer to resize.
        size_t new_size ///< [in] New buffer size in bytes.
);

/**
 * @brief Clear buffer: release memory, reset fields to default values.
 *
 * Buffer memory is freed only if it is the buffer's own memory.
 */
void
station_clear_buffer(
        struct station_buffer *buffer ///< [in] Buffer to clear.
);

#endif // _STATION_BUFFER_FUN_H_

