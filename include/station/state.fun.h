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
 * @brief Operations on finite state machine states.
 */

#pragma once
#ifndef _STATION_STATE_FUN_H_
#define _STATION_STATE_FUN_H_

#include <station/index.typ.h>

struct station_state;

/**
 * @brief Allocate FSM state with the specified number of subsequent states.
 *
 * @return FSM state.
 */
struct station_state*
station_alloc_state(
        station_states_number_t num_next_states ///< Number of next states for the current state.
);

#endif // _STATION_STATE_FUN_H_

