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
 * @brief Types for fonts.
 */

#pragma once
#ifndef _STATION_FONT_TYP_H_
#define _STATION_FONT_TYP_H_

#include <stdint.h>

/**
 * @brief Header of PC Screen Font version 2.
 */
typedef struct station_font_psf2_header {
    uint32_t magic;           ///< Magic bytes to identify PSF.
    uint32_t version;         ///< Zero.
    uint32_t header_size;     ///< Offset of bitmaps in file, 32.
    uint32_t flags;           ///< 1 if there's unicode table, 0 otherwise.
    uint32_t num_glyphs;      ///< Number of glyphs.
    uint32_t bytes_per_glyph; ///< Size of each glyph.
    uint32_t height;          ///< Height in pixels.
    uint32_t width;           ///< Width in pixels.
} station_font_psf2_header_t;

/**
 * @brief PC Screen Font version 2, representation in memory.
 */
typedef struct station_font_psf2 {
    station_font_psf2_header_t *header; ///< Font header.
    unsigned char *glyphs;              ///< Font glyphs.

    uint32_t *mapping_table; ///< (Unicode code point) -> (glyph index) mapping table.
} station_font_psf2_t;

#endif // _STATION_FONT_TYP_H_

