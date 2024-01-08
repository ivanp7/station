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
 * @brief Constants and macros for finite state machine execution.
 */

#pragma once
#ifndef _STATION_FSM_DEF_H_
#define _STATION_FSM_DEF_H_

/**
 * @brief Finite state machine execution result: success.
 */
#define STATION_FSM_EXEC_SUCCESS 0
/**
 * @brief Finite state machine execution result: inputs are incorrect.
 */
#define STATION_FSM_EXEC_INCORRECT_INPUTS 1
/**
 * @brief Finite state machine execution result: malloc() failure.
 */
#define STATION_FSM_EXEC_MALLOC_FAIL 2
/**
 * @brief Finite state machine execution result: thrd_create() failure - thrd_nomem.
 */
#define STATION_FSM_EXEC_THRD_NOMEM 3
/**
 * @brief Finite state machine execution result: thrd_create() failure - thrd_error.
 */
#define STATION_FSM_EXEC_THRD_ERROR 4

/**
 * @brief Finite state machine execution result: SDL is not supported.
 */
#define STATION_FSM_EXEC_SDL_NOT_SUPPORTED 5
/**
 * @brief Finite state machine execution result: SDL_Init() failure.
 */
#define STATION_FSM_EXEC_SDL_INIT_FAIL 6
/**
 * @brief Finite state machine execution result: SDL_CreateWindow() failure.
 */
#define STATION_FSM_EXEC_SDL_CREATE_WINDOW_FAIL 7
/**
 * @brief Finite state machine execution result: SDL_CreateRenderer() failure.
 */
#define STATION_FSM_EXEC_SDL_CREATE_RENDERER_FAIL 8
/**
 * @brief Finite state machine execution result: SDL_CreateTexture() failure.
 */
#define STATION_FSM_EXEC_SDL_CREATE_TEXTURE_FAIL 9

#endif // _STATION_FSM_DEF_H_

