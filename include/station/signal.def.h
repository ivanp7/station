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
 * @brief Constants and macros for signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_DEF_H_
#define _STATION_SIGNAL_DEF_H_

#include <stdbool.h>

/**
 * @brief Generate declarations for signal management interface.
 *
 * @note This macro is to be used in a header file.
 *
 * station_signal_management_watch_SIGNAL() sets the custom handler for the SIGNAL, so that
 * its state (was it raised or not) can be observed via station_signal_raised_SIGNAL() and
 * reset via station_signal_reset_SIGNAL().
 *
 * station_signal_management_ignore_SIGNAL() makes SIGNAL ignored.
 * station_signal_management_default_SIGNAL() restores SIGNAL action to the default behavior.
 */
#define STATION_SIGNAL_MANAGEMENT_DECLARATION(signal)       \
    void station_signal_reset_##signal(void);               \
    bool station_signal_raised_##signal(void);              \
    bool station_signal_management_watch_##signal(void);    \
    bool station_signal_management_ignore_##signal(void);   \
    bool station_signal_management_default_##signal(void);

/**
 * @brief Generate definitions for signal management interface.
 *
 * @note This macro is to be used together with
 * STATION_SIGNAL_SUPPORT_DECLARATION() for the same signal.
 *
 * @note This macro depends on <signal.h> and <stdatomic.h>.
 */
#define STATION_SIGNAL_MANAGEMENT_DEFINITION(signal)                                \
    static volatile atomic_bool station_signal_flag_##signal;                       \
                                                                                    \
    void station_signal_reset_##signal(void) {                                      \
        atomic_store_explicit(&station_signal_flag_##signal, false,                 \
                memory_order_release); }                                            \
                                                                                    \
    bool station_signal_raised_##signal(void) {                                     \
        return atomic_load_explicit(&station_signal_flag_##signal,                  \
                memory_order_acquire); }                                            \
                                                                                    \
    static void station_signal_handler_##signal(int signo) {                        \
        (void) signo;                                                               \
        atomic_store_explicit(&station_signal_flag_##signal, true,                  \
                memory_order_release); }                                            \
                                                                                    \
    bool station_signal_management_watch_##signal(void) {                           \
        if (!atomic_is_lock_free(&station_signal_flag_##signal)) return false;      \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = station_signal_handler_##signal};     \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }           \
                                                                                    \
    bool station_signal_management_ignore_##signal(void) {                          \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = SIG_IGN};                             \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }           \
                                                                                    \
    bool station_signal_management_default_##signal(void) {                         \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = SIG_DFL};                             \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }

#endif // _STATION_SIGNAL_DEF_H_

