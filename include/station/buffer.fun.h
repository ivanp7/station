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

struct station_buffer;

/**
 * @brief Read contents of a file into a newly created buffer.
 *
 * @return Buffer, or NULL in case of failure.
 */
struct station_buffer*
station_create_buffer_from_file(
        const char *file ///< [in] Path to file to read data from.
);

/**
 * @brief Destroy a buffer.
 */
void
station_destroy_buffer(
        struct station_buffer *buffer ///< [in] Buffer to destroy.
);

#endif // _STATION_BUFFER_FUN_H_

