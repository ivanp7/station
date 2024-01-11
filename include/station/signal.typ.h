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
 * @brief Types for signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_TYP_H_
#define _STATION_SIGNAL_TYP_H_

#include <stdbool.h>

/**
 * @brief States of supported signals.
 */
typedef struct station_signal_states {
    bool raised_SIGHUP;
    bool raised_SIGINT;
    bool raised_SIGQUIT;
    bool raised_SIGUSR1;
    bool raised_SIGUSR2;
    bool raised_SIGALRM;
    bool raised_SIGTERM;
    bool raised_SIGTSTP;
    bool raised_SIGTTIN;
    bool raised_SIGTTOU;
    bool raised_SIGWINCH;
} station_signal_states_t;

#endif // _STATION_SIGNAL_TYP_H_

