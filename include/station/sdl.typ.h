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
 * @brief Types for SDL support.
 */

#pragma once
#ifndef _STATION_SDL_TYP_H_
#define _STATION_SDL_TYP_H_

#include <stdint.h>
#include <stdbool.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

/**
 * @brief Properties of a created SDL window.
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct station_sdl_window_properties {
    struct {
        uint32_t width;  ///< Texture width in pixels.
        uint32_t height; ///< Texture height in pixels.
    } texture;

    struct {
        uint32_t width;  ///< Window width in pixels.
        uint32_t height; ///< Window height in pixels.

        uint32_t flags; ///< Window flags.

        const char *title; ///< Window title.
    } window;
} station_sdl_window_properties_t;

/**
 * @brief SDL window context.
 */
typedef struct station_sdl_window_context {
    struct {
        struct SDL_Window *handle; ///< SDL window handle.
    } window;

    struct {
        struct SDL_Renderer *handle; ///< SDL renderer handle.
    } renderer;

    struct {
        struct SDL_Texture *handle; ///< SDL texture handle.
        uint32_t width;  ///< SDL texture width in pixels.
        uint32_t height; ///< SDL texture height in pixels.

        struct {
            const struct SDL_Rect *rectangle; ///< Locked texture pixels rectangle.
            uint32_t *pixels; ///< Pointer to locked pixels of the texture.
            uint32_t pitch;   ///< Size of a full texture row in pixels.
        } lock;
    } texture;
} station_sdl_window_context_t;

#endif // _STATION_SDL_TYP_H_

