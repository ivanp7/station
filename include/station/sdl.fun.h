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
 * @brief SDL related functions.
 */

#pragma once
#ifndef _STATION_SDL_FUN_H_
#define _STATION_SDL_FUN_H_

struct station_sdl_window_context;
struct station_sdl_window_properties;
struct SDL_Rect;

/**
 * @brief Create SDL window with accompanying resources - renderer and texture.
 *
 * Steps:
 * 1. SDL_CreateWindow()
 * 2. SDL_CreateRenderer()
 * 3. SDL_CreateTexture()
 *
 * @return 0 if succeed, -1 if arguments are incorrect,
 * -2 if SDL not supported, otherwise number of the failed step.
 */
int
station_sdl_initialize_window_context(
        struct station_sdl_window_context *context, ///< [out] Context to initialize.
        const struct station_sdl_window_properties *properties ///< [in] Window properties.
);

/**
 * @brief Destroy SDL window.
 *
 * Steps:
 * 1. SDL_DestroyTexture()
 * 2. SDL_DestroyRenderer()
 * 3. SDL_DestroyWindow()
 */
void
station_sdl_destroy_window_context(
        struct station_sdl_window_context *context ///< [in,out] Context to destroy.
);

/**
 * @brief Lock window texture for update.
 *
 * @return 0 if succeed, -1 if context is invalid,
 * -2 if SDL not supported, 2 if texture is locked already, otherwise 1.
 */
int
station_sdl_window_lock_texture(
        struct station_sdl_window_context *context, ///< [in,out] Context.
        const struct SDL_Rect *rectangle ///< [in] Rectangle to lock, or NULL for full texture.
);

/**
 * @brief Unlock window texture and render updated rectangle.
 *
 * @return 0 if succeed, -1 if context is invalid,
 * -2 if SDL not supported, 2 if texture is not locked, otherwise 1.
 */
int
station_sdl_window_unlock_texture_and_render(
        struct station_sdl_window_context *context ///< [in,out] Context.
);

#endif // _STATION_SDL_FUN_H_

