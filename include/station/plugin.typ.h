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
#ifndef _STATION_PLUGIN_TYP_H_
#define _STATION_PLUGIN_TYP_H_

#include <station/index.typ.h>

struct station_state;
struct station_sdl_properties;
struct station_sdl_context;
struct station_opencl_context;
struct station_signal_set;

/**
 * @brief Plugin help function.
 *
 * This function must do nothing besides argument parsing and displaying plugin usage help.
 *
 * @return Application exit code.
 */
typedef int (*station_plugin_help_func_t)(
        int argc,    ///< [in] Number of command line arguments.
        char *argv[] ///< [in] Command line arguments.
);

/**
 * @brief Plugin initialization function.
 *
 * @warning Do not create any threads in this function,
 * or initialize any resources that can create threads!
 *
 * This function must initialize plugin:
 * create resources, set initial FSM state,
 * set number of parallel processing threads,
 * and set SDL-related FSM properties (only when a window is to be created).
 *
 * SDL context pointer is not NULL when a window is to be created.
 * SDL context is not to be modified, it is for storage only,
 * so that is can be accessed from state functions later.
 *
 * If the returned value is not zero, execution is terminated immediately, and
 * the finalization function won't be called.
 *
 * @return Application exit code.
 */
typedef int (*station_plugin_init_func_t)(
        void **plugin_resources, ///< [out] Plugin resources.

        struct station_state *initial_state, ///< [out] Initial state of finite state machine.
        void **fsm_data,                     ///< [out] Finite state machine data.

        station_threads_number_t *num_threads,         ///< [in,out] Number of parallel processing threads.
        struct station_signal_set *signals,            ///< [in,out] States of supported signals.
        struct station_sdl_properties *sdl_properties, ///< [in,out] SDL-related properties of a FSM.

        struct station_sdl_context *future_sdl_context,       ///< [in] Pointer to future SDL context.
        struct station_opencl_context *future_opencl_context, ///< [in] Pointer to future OpenCL context.

        int argc,    ///< [in] Number of command line arguments.
        char *argv[] ///< [in] Command line arguments.
);

/**
 * @brief Plugin finalization function.
 *
 * This function is supposed to release plugin resources.
 *
 * @return Application exit code.
 */
typedef int (*station_plugin_final_func_t)(
        void *plugin_resources ///< [in] Plugin resources created by the plugin initialization function.
);

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

#endif // _STATION_PLUGIN_TYP_H_

