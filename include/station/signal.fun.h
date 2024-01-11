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
 * @brief Signal management functions.
 */

#pragma once
#ifndef _STATION_SIGNAL_FUN_H_
#define _STATION_SIGNAL_FUN_H_

#include <station/signal.def.h>
#include <station/func.def.h>

struct station_state;
struct station_fsm_context;

// Supported signals
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGHUP)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGINT)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGQUIT)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGUSR1)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGUSR2)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGALRM)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGTERM)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGTSTP)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGTTIN)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGTTOU)
STATION_SIGNAL_MANAGEMENT_DECLARATION(SIGWINCH)

/**
 * @brief State function that checks and updates signal states.
 *
 * This state function expects station_state_chain_t as state data,
 * with .data field of it being of type station_signal_states_t.
 */
STATION_SFUNC(station_signal_management_sfunc);

#endif // _STATION_SIGNAL_FUN_H_

