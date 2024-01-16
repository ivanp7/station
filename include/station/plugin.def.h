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
#define STATION_PLUGIN_VERSION 20240115

/**
 * @brief Name of plugin format structure object.
 */
#define STATION_PLUGIN_FORMAT_OBJECT station_plugin_format_object
/**
 * @brief Name of plugin vtable structure object.
 */
#define STATION_PLUGIN_VTABLE_OBJECT station_plugin_vtable_object

/**
 * @brief Plugin help function declarator.
 *
 * @see station_plugin_help_func_t
 */
#define STATION_PLUGIN_HELP_FUNC(name) \
    void name(int argc, char *argv[])

/**
 * @brief Plugin configuration function declarator.
 *
 * @see station_plugin_conf_func_t
 */
#define STATION_PLUGIN_CONF_FUNC(name) \
    void name(station_plugin_conf_func_args_t *args, int argc, char *argv[])

/**
 * @brief Plugin initialization function declarator.
 *
 * @see station_plugin_init_func_t
 */
#define STATION_PLUGIN_INIT_FUNC(name) \
    void name(const station_plugin_init_func_inputs_t *inputs, \
            station_plugin_init_func_outputs_t *outputs)

/**
 * @brief Plugin finalization function declarator.
 *
 * @see station_plugin_final_func_t
 */
#define STATION_PLUGIN_FINAL_FUNC(name) \
    int name(void *plugin_resources, bool quick)

/**
 * @brief Define plugin objects.
 */
#define STATION_PLUGIN(plugin_name, plugin_help, plugin_conf, plugin_init, plugin_final) \
    station_plugin_format_t STATION_PLUGIN_FORMAT_OBJECT = {    \
        .signature = STATION_PLUGIN_SIGNATURE,                  \
        .version = STATION_PLUGIN_VERSION};                     \
    station_plugin_vtable_t STATION_PLUGIN_VTABLE_OBJECT = {    \
        .name = plugin_name, .help_fn = plugin_help,            \
        .conf_fn = plugin_conf, .init_fn = plugin_init,         \
        .final_fn = plugin_final};

#endif // _STATION_PLUGIN_DEF_H_

