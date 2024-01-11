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
#ifndef _STATION_PLUGIN_DEF_H_
#define _STATION_PLUGIN_DEF_H_

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
#define STATION_PLUGIN_PREAMBLE()                                   \
    static int station_plugin_help(int argc, char *argv[]);         \
    static void* station_plugin_init(                               \
            struct station_state *initial_state,                    \
            station_threads_number_t *num_threads,                  \
            struct station_sdl_properties *sdl_properties,          \
            struct station_sdl_context *sdl_context,                \
            struct station_opencl_context *opencl_context,          \
            int argc, char *argv[]);                                \
    static int station_plugin_final(void *plugin_resources);        \
    station_plugin_format_t STATION_PLUGIN_FORMAT_OBJECT = {        \
        .signature = STATION_PLUGIN_SIGNATURE,                      \
        .version = STATION_PLUGIN_VERSION};                         \
    station_plugin_vtable_t STATION_PLUGIN_VTABLE_OBJECT = {        \
        .help_fn = station_plugin_help,                             \
        .init_fn = station_plugin_init,                             \
        .final_fn = station_plugin_final};

/**
 * @brief Implement plugin help function.
 */
#define STATION_PLUGIN_HELP(argc, argv) \
    static int station_plugin_help(int argc, char *argv[])

/**
 * @brief Implement plugin initialization function.
 */
#define STATION_PLUGIN_INIT(initial_state, num_threads, sdl_properties, sdl_context, opencl_context, argc, argv) \
    static void* station_plugin_init(                           \
            station_state_t *initial_state,                     \
            station_threads_number_t *num_threads,              \
            station_sdl_properties_t *sdl_properties,           \
            struct station_sdl_context *sdl_context,            \
            struct station_opencl_context *opencl_context,      \
            int argc, char *argv[])

/**
 * @brief Implement plugin finalization function.
 */
#define STATION_PLUGIN_FINAL(plugin_resources) \
    static int station_plugin_final(void *plugin_resources)

#endif // _STATION_PLUGIN_DEF_H_

