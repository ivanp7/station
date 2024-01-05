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
 * @brief Types of finite state machine states.
 */

#pragma once
#ifndef _STATION_STATE_TYP_H_
#define _STATION_STATE_TYP_H_

#include <station/func.typ.h>

/**
 * @brief Finite state machine state.
 */
typedef struct station_state {
    station_sfunc_t sfunc; ///< State function.
    void *state_data;      ///< State data.

    struct station_state* next_state[]; ///< Array of possible subsequent states.
} station_state_t;

#endif // _STATION_STATE_TYP_H_

