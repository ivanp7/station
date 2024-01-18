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

#include <stdint.h>
#include <stdbool.h>

struct station_sdl_window_context;
struct station_sdl_window_properties;

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
 * If whole_texture is true, parameters x, y, width, height are ignored.
 *
 * @return 0 if succeed, -1 if context is invalid,
 * -2 if SDL not supported, 2 if texture is locked already, otherwise 1.
 */
int
station_sdl_window_lock_texture(
        struct station_sdl_window_context *context, ///< [in,out] Context.

        bool whole_texture, ///< [in] Whether to lock full texture.

        uint32_t x, ///< [in] X coordinate of rectangle to lock.
        uint32_t y, ///< [in] Y coordinate of rectangle to lock.
        uint32_t width, ///< [in] Width of rectangle to lock.
        uint32_t height ///< [in] Height of rectangle to lock.
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

/**
 * @brief Draw glyph on window texture.
 *
 * Glyph rows must always begin at byte border.
 *
 * @return True if texture has been modified, otherwise false.
 */
bool
station_sdl_window_texture_draw_glyph(
        struct station_sdl_window_context *context, ///< [in,out] Context.

        int32_t x, ///< [in] X coordinate of glyph's upper left corner.
        int32_t y, ///< [in] Y coordinate of glyph's upper left corner.

        bool draw_fg, ///< [in] Whether to draw foreground pixels.
        bool draw_bg, ///< [in] Whether to draw foreground pixels.

        uint32_t fg, ///< [in] Glyph foreground color.
        uint32_t bg, ///< [in] Glyph background color.

        const unsigned char *glyph, ///< [in] Glyph to draw.
        uint32_t glyph_width,       ///< [in] Glyph width in pixels.
        uint32_t glyph_height,      ///< [in] Glyph height in pixels.

        int32_t glyph_col_idx, ///< [in] Index of the first glyph column to draw.
        int32_t glyph_row_idx, ///< [in] Index of the first glyph row to draw.
        int32_t glyph_num_cols, ///< [in] Number of glyph columns to draw.
        int32_t glyph_num_rows  ///< [in] Number of glyph rows to draw.
);

#endif // _STATION_SDL_FUN_H_

