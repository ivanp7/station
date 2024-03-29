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
 * @brief Signal management functions.
 */

#pragma once
#ifndef _STATION_SIGNAL_FUN_H_
#define _STATION_SIGNAL_FUN_H_

#include <station/signal.typ.h>

struct station_signal_management_context;

/**
 * @brief Start signal management thread.
 *
 * @warning There should be only one signal management thread per application,
 * which must be created in the main thread before any other thread,
 * otherwise signal management won't work (reliably or at all).
 *
 * Input set of signals specifies which signals are to be watched
 * (signals, for which the corresponding flags are true).
 *
 * Before the thread is started, all flags are cleared (reset to false).
 * When a signal is caught, its corresponding flag is set to true.
 *
 * Signal handler argument is optional. This handler is called synchronously
 * and used to get access to the siginfo_t value.
 *
 * @return Signal management context.
 */
struct station_signal_management_context*
station_signal_management_thread_start(
        station_std_signal_set_t *std_signals, ///< [in,out] Standard signals to catch.
        station_rt_signal_set_t *rt_signals,   ///< [in,out] Real-time signals to catch.
        station_signal_handler_func_t signal_handler, ///< [in] Signal handler.
        void *signal_handler_data ///< [in] Signal handler data.
);

/**
 * @brief Stop signal management thread.
 */
void
station_signal_management_thread_stop(
        struct station_signal_management_context *context ///< [in] Signal management context.
);

/**
 * @brief Extract signal management thread properties.
 */
void
station_signal_management_thread_get_properties(
        struct station_signal_management_context *context, ///< [in] Signal management context.

        station_std_signal_set_t **std_signals, ///< [out] Standard signals to catch.
        station_rt_signal_set_t **rt_signals,   ///< [out] Real-time signals to catch.
        station_signal_handler_func_t *signal_handler, ///< [out] Signal handler.
        void **signal_handler_data ///< [out] Signal handler data.
);

#endif // _STATION_SIGNAL_FUN_H_

