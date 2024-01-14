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

#include <station/state.typ.h>
#include <stdbool.h>

struct station_signal_set;
struct station_sdl_properties;
struct station_sdl_context;
struct station_opencl_context;

/**
 * @brief Arguments for plugin initialization function.
 *
 * Future SDL context pointer is NULL if SDL is not supported.
 * Future OpenCL context pointer is NULL if OpenCL is not supported.
 *
 * Pointers to future SDL and OpenCL contexts are to be copied if needed.
 * Contents of future SDL and OpenCL contexts should not be accessed/modified
 * from the initialization function.
 */
typedef struct station_plugin_init_func_args {
    void *plugin_resources; ///< Plugin resources.

    station_state_t fsm_initial_state; ///< Initial state of finite state machine.
    void *fsm_data; ///< Finite state machine data (argument passed to all state functions).
    station_threads_number_t fsm_num_threads; ///< Number of parallel processing threads.

    bool signals_not_needed; ///< Whether signal management is not used and should be disabled.
    struct station_signal_set *signals; ///< States of supported signals.

    bool sdl_not_needed; ///< Whether SDL support is not used and should be disabled.
    struct station_sdl_properties *sdl_properties; ///< SDL-related properties of a finite state machine.
    struct station_sdl_context *future_sdl_context; ///< Pointer to future SDL context (invalid as of initialization).

    bool opencl_not_needed; ///< Whether OpenCL support is not used and should be disabled.
    struct station_opencl_context *future_opencl_context; ///< Pointer to future OpenCL context (invalid as of initialization).
} station_plugin_init_func_args_t;

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
 * or initialize any resources that can create threads,
 * otherwise signal management will break!
 *
 * This function must allocate plugin resources and set fields
 * of the arguments structure to the corresponding values.
 *
 * If the returned value is zero, execution is continued normally.
 * If the returned value is not zero, execution is terminated immediately,
 * and the plugin finalization function won't be called.
 *
 * @return Application exit code.
 */
typedef int (*station_plugin_init_func_t)(
        station_plugin_init_func_args_t *args, ///< [in,out] Structure with function arguments.
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

