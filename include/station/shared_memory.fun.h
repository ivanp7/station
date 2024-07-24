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
 * @brief Shared memory operations.
 */

#pragma once
#ifndef _STATION_SHARED_MEMORY_FUN_H_
#define _STATION_SHARED_MEMORY_FUN_H_

/**
 * @brief Attach shared memory segment with pointer support.
 *
 * The pointer support is implemented by storing original shmaddr
 * (address of the memory in the creator process)
 * as the first object in the shared memory segment.
 * The original shmaddr is read and the segment is reattached to that address,
 * making pointers in the shared memory valid.
 *
 * @return Pointer to shared memory or NULL in case of error.
 */
void*
station_shared_memory_attach_with_ptr_support(
        int shmid, ///< [in] Shared memory segment identifier.
        int shmflg ///< [in] Attached memory flags.
);

#endif // _STATION_SHARED_MEMORY_FUN_H_

