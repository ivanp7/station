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

/**
 * @brief Generate declarations for a convenient signal processing interface.
 *
 * @note This macro is to be used in a header file.
 *
 * @note This macro depends on <stdbool.h>.
 *
 * station_signal_handler_watch_SIGNAL() sets the custom handler for the SIGNAL, so that
 * its state (was it raised or not) can be observed via station_signal_raised_SIGNAL() and
 * reset via station_signal_reset_SIGNAL().
 *
 * station_signal_handler_ignore_SIGNAL() makes SIGNAL ignored.
 * station_signal_handler_default_SIGNAL() restores SIGNAL action to the default behavior.
 */
#define STATION_SIGNAL_SUPPORT_DECLARATION(signal)      \
    void station_signal_reset_##signal(void);           \
    bool station_signal_raised_##signal(void);          \
    bool station_signal_handler_watch_##signal(void);   \
    bool station_signal_handler_ignore_##signal(void);  \
    bool station_signal_handler_default_##signal(void);

/**
 * @brief Generate definitions for a convenient signal processing interface.
 *
 * @note This macro is to be used in a source file,
 * which must include a header with STATION_SIGNAL_SUPPORT_DECLARATION() of the same signal.
 *
 * @note This macro depends on <signal.h> and <stdatomic.h>.
 */
#define STATION_SIGNAL_SUPPORT_DEFINITION(signal)                                   \
    static atomic_bool station_signal_flag##signal;                                 \
                                                                                    \
    void station_signal_reset_##signal(void) {                                      \
        atomic_store_explicit(&station_signal_flag##signal, false,                  \
                memory_order_release); }                                            \
                                                                                    \
    bool station_signal_raised_##signal(void) {                                     \
        return atomic_load_explicit(&station_signal_flag##signal,                   \
                memory_order_acquire); }                                            \
                                                                                    \
    static void station_signal_handler_##signal(int signo) {                        \
        if (signo != signal) return;                                                \
        atomic_store_explicit(&station_signal_flag##signal, true,                   \
                memory_order_release); }                                            \
                                                                                    \
    bool station_signal_handler_watch_##signal(void) {                              \
        if (!atomic_is_lock_free(&station_signal_flag##signal)) return false;       \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = station_signal_handler_##signal};     \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }           \
                                                                                    \
    bool station_signal_handler_ignore_##signal(void) {                             \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = SIG_IGN};                             \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }           \
                                                                                    \
    bool station_signal_handler_default_##signal(void) {                            \
        station_signal_reset_##signal();                                            \
        struct sigaction act = {.sa_handler = SIG_DFL};                             \
        return (sigaction(signal, &act, (struct sigaction*)NULL) == 0); }

#endif // _STATION_SIGNAL_DEF_H_

