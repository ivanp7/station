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
 * @brief Types for finite state machines.
 */

#pragma once
#ifndef _STATION_FSM_TYP_H_
#define _STATION_FSM_TYP_H_

struct station_state;

/**
 * @brief State function of a finite state machine.
 *
 * State function is supposed to modify its 'state' argument, setting it to the next state.
 * If the next state function is NULL, then FSM execution is terminated.
 * The FSM execution loop is like the following:
 *
 * while (state.sfunc != NULL)
 *     state.sfunc(&state, fsm_data);
 */
typedef void (*station_sfunc_t)(
        struct station_state *state, ///< [in,out] Current state (as input), next state (as output).
        void *fsm_data ///< [in,out] Finite state machine data.
);

/**
 * @brief Finite state machine state.
 */
typedef struct station_state {
    station_sfunc_t sfunc; ///< State function.
    void *data; ///< State data.
} station_state_t;

/**
 * @brief Chain (linked list) of finite state machine states.
 */
typedef struct station_state_chain {
    station_state_t next_state; ///< Next state.
    void *current_data; ///< Current state data.
} station_state_chain_t;

#endif // _STATION_FSM_TYP_H_

