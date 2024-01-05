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
 * @brief Constants and macros for application plugins.
 */

#pragma once
#ifndef _STATION_APP_PLUGIN_DEF_H_
#define _STATION_APP_PLUGIN_DEF_H_

/**
 * @brief Plugin signature - value uniquely identifying plugin format.
 */
#define STATION_PLUGIN_SIGNATURE 0xfeedDEAD
/**
 * @brief Plugin version - value determining application-plugin compatibility.
 */
#define STATION_PLUGIN_VERSION 20240101

/**
 * @brief Name of plugin format structure object.
 */
#define STATION_PLUGIN_FORMAT_OBJECT station_plugin_format_object
/**
 * @brief Name of plugin vtable structure object.
 */
#define STATION_PLUGIN_VTABLE_OBJECT station_plugin_vtable_object

/**
 * @brief Plugin preamble.
 *
 * Declares functions, defines global objects.
 */
#define STATION_PLUGIN_PREAMBLE()                                       \
    void station_plugin_help(int argc, const char *argv[]);             \
    bool station_plugin_init(station_plugin_context_t *plugin_context,  \
            struct station_sdl_context *sdl_context,                    \
            int argc, const char *argv[]);                              \
    bool station_plugin_final(void *resources);                         \
    station_plugin_format_t STATION_PLUGIN_FORMAT_OBJECT = {            \
        .signature = STATION_PLUGIN_SIGNATURE,                          \
        .version = STATION_PLUGIN_VERSION};                             \
    station_plugin_vtable_t STATION_PLUGIN_VTABLE_OBJECT = {            \
        .help_fn = station_plugin_help,                                 \
        .init_fn = station_plugin_init,                                 \
        .final_fn = station_plugin_final};

/**
 * @brief Implement plugin help function.
 */
#define STATION_PLUGIN_HELP(argc, argv) \
    void station_plugin_help(int argc, const char *argv[])

/**
 * @brief Implement plugin initialization function.
 */
#define STATION_PLUGIN_INIT(plugin_context, sdl_context, argc, argv)    \
    bool station_plugin_init(station_plugin_context_t *plugin_context,  \
            struct station_sdl_context *sdl_context,                    \
            int argc, const char *argv[])

/**
 * @brief Implement plugin finalization function.
 */
#define STATION_PLUGIN_FINAL(resources) \
    bool station_plugin_final(void *resources)

#endif // _STATION_APP_PLUGIN_DEF_H_

