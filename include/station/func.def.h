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
 * @brief Finite state machine function macros.
 */

#pragma once
#ifndef _STATION_FUNC_DEF_H_
#define _STATION_FUNC_DEF_H_

/**
 * @brief Declarator of a state function of a finite state machine.
 */
#define STATION_SFUNC(name) \
    void name(struct station_state *state, void *fsm_data, struct station_fsm_context *fsm_context)

/**
 * @brief Declarator of a parallel processing function of a finite state machine.
 */
#define STATION_PFUNC(name) \
    void name(void *data, station_task_idx_t task_idx, station_thread_idx_t thread_idx)

#endif // _STATION_FUNC_DEF_H_

