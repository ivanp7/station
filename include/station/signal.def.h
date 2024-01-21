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
 * @brief Macros for signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_DEF_H_
#define _STATION_SIGNAL_DEF_H_

/**
 * @brief Check state of a signal flag.
 */
#define STATION_SIGNAL_IS_FLAG_SET(flag_ptr) \
    atomic_load_explicit(flag_ptr, memory_order_acquire)

/**
 * @brief Set a signal flag.
 */
#define STATION_SIGNAL_SET_FLAG(flag_ptr) \
    atomic_store_explicit(flag_ptr, true, memory_order_release)

/**
 * @brief Unset a signal flag.
 */
#define STATION_SIGNAL_UNSET_FLAG(flag_ptr) \
    atomic_store_explicit(flag_ptr, false, memory_order_release)

#endif // _STATION_SIGNAL_DEF_H_

