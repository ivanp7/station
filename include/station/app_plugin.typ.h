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
 * @brief Types for application plugins.
 */

#pragma once
#ifndef _STATION_APP_PLUGIN_TYP_H_
#define _STATION_APP_PLUGIN_TYP_H_

#include <station/index.typ.h>
#include <stdbool.h>

struct station_state;
struct station_sdl_context;

/**
 * @brief Application plugin context.
 */
typedef struct station_plugin_context {
    void *resources; ///< Plugin resources.

    struct station_state *initial_state;  ///< Initial state of finite state machine.
    station_threads_number_t num_threads; ///< Number of parallel processing threads.

    uint32_t sdl_init_flags; ///< Flags to pass to SDL_Init() call.

    uint16_t sdl_texture_width;  ///< SDL texture width in pixels.
    uint16_t sdl_texture_height; ///< SDL texture height in pixels.

    const char *sdl_window_title; ///< SDL window title.
} station_plugin_context_t;

/**
 * @brief Plugin help function.
 *
 * This function must do nothing besides argument parsing and displaying plugin usage help.
 */
typedef void (*station_plugin_help_func_t)(int argc, const char *argv[]);

/**
 * @brief Plugin initialization function.
 *
 * This function must initialize plugin context -- create resources,
 * construct finite state machine, optionally set other context fields.
 * SDL context is not to be modified, it is for storage only.
 * SDL context pointer is not NULL when a window is to be created.
 *
 * @return True on success, false on fail.
 */
typedef bool (*station_plugin_init_func_t)(
        station_plugin_context_t *plugin_context,
        struct station_sdl_context *sdl_context,
        int argc, const char *argv[]);

/**
 * @brief Plugin finalization function.
 *
 * This function must release plugin resources.
 *
 * @return True on success, false on fail.
 */
typedef bool (*station_plugin_final_func_t)(void *resources);

/**
 * @brief Plugin format.
 */
typedef struct station_plugin_format {
    uint32_t signature; ///< Value uniquely identifying plugin format.
    uint32_t version;   ///< Value determining application-plugin compatibility.
} station_plugin_format_t;

/**
 * @brief Plugin vtable.
 */
typedef struct station_plugin_vtable {
    station_plugin_help_func_t help_fn;   ///< Pointer to plugin help function.
    station_plugin_init_func_t init_fn;   ///< Pointer to plugin initialization function.
    station_plugin_final_func_t final_fn; ///< Pointer to plugin finalization function.
} station_plugin_vtable_t;

#endif // _STATION_APP_PLUGIN_TYP_H_

