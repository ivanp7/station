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
 * @brief Types for finite state machines with SDL support.
 */

#pragma once
#ifndef _STATION_SDL_TYP_H_
#define _STATION_SDL_TYP_H_

#include <stdint.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

/**
 * @brief SDL-related properties of a finite state machine.
 */
typedef struct station_sdl_properties {
    uint32_t sdl_init_flags; ///< Flags to pass to SDL_Init() call.

    uint16_t texture_width;  ///< Texture width in pixels.
    uint16_t texture_height; ///< Texture height in pixels.

    uint16_t window_width;  ///< Window width in pixels.
    uint16_t window_height; ///< Window height in pixels.

    uint8_t window_shown;     ///< Window is shown upon creation.
    uint8_t window_resizable; ///< Window is resizable.

    const char *window_title; ///< Window title.
} station_sdl_properties_t;

/**
 * @brief SDL context for a finite state machine.
 */
typedef struct station_sdl_context {
    struct SDL_Window *window;     ///< SDL window handle.
    struct SDL_Renderer *renderer; ///< SDL renderer handle.
    struct SDL_Texture *texture;   ///< SDL texture handle.

    const struct SDL_Rect *texture_lock_rectangle; ///< Locked texture pixels rectangle.
    uint32_t *texture_lock_pixels; ///< Pointer to locked pixels of the texture.
    uint32_t texture_lock_pitch;   ///< Length of a full texture row in pixels.

    uint32_t texture_width;  ///< SDL texture width in pixels.
    uint32_t texture_height; ///< SDL texture height in pixels.
} station_sdl_context_t;

#endif // _STATION_SDL_TYP_H_

