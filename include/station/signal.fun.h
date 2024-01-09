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
 * @brief Signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_FUN_H_
#define _STATION_SIGNAL_FUN_H_

#include <station/signal.def.h>
#include <stdbool.h>

STATION_SIGNAL_SUPPORT_DECLARATION(SIGHUP)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGINT)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGQUIT)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGUSR1)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGUSR2)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGALRM)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGTERM)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGTSTP)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGTTIN)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGTTOU)
STATION_SIGNAL_SUPPORT_DECLARATION(SIGWINCH)

#endif // _STATION_SIGNAL_FUN_H_

