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
 * @brief Font-related functions.
 */

#pragma once
#ifndef _STATION_FONT_FUN_H_
#define _STATION_FONT_FUN_H_

#include <stddef.h>

struct station_font_psf2;
struct station_buffer;

/**
 * @brief Load PC Screen Font version 2 from buffer.
 *
 * @return Font.
 */
struct station_font_psf2*
station_load_font_psf2_from_buffer(
        const struct station_buffer *buffer ///< [in] Buffer with font data.
);

/**
 * @brief Unload PC Screen Font version 2.
 */
void
station_unload_font_psf2(
        struct station_font_psf2 *font ///< [in] Font to unload.
);

/**
 * @brief Get glyph (PC Screen Font version 2) of a character.
 *
 * @return Character glyph.
 */
const unsigned char*
station_font_psf2_glyph(
        const char *utf8_str, ///< [in] UTF-8 string.
        size_t utf8_str_len,  ///< [in] Length of UTF-8 string in bytes.

        size_t *chr_len, ///< [out] Length of the first character in UTF-8 string in bytes.
        const struct station_font_psf2 *font ///< [in] Font.
);

#endif // _STATION_FONT_FUN_H_

