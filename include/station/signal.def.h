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
 * @brief Macros for signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_DEF_H_
#define _STATION_SIGNAL_DEF_H_

/**
 * @brief Check state of a signal flag.
 */
#define STATION_SIGNAL_IS_FLAG_SET(flag_ptr) \
    atomic_load_explicit(flag_ptr, memory_order_relaxed)

/**
 * @brief Set a signal flag.
 */
#define STATION_SIGNAL_SET_FLAG(flag_ptr) \
    atomic_store_explicit(flag_ptr, true, memory_order_relaxed)

/**
 * @brief Unset a signal flag.
 */
#define STATION_SIGNAL_UNSET_FLAG(flag_ptr) \
    atomic_store_explicit(flag_ptr, false, memory_order_relaxed)

/**
 * @brief Signal set with all flags raised.
 */
#define STATION_STD_SIGNAL_SET_ALL  \
    (station_std_signal_set_t){     \
        .signal_SIGINT = true,      \
        .signal_SIGQUIT = true,     \
        .signal_SIGTERM = true,     \
                                    \
        .signal_SIGCHLD = true,     \
        .signal_SIGCONT = true,     \
        .signal_SIGTSTP = true,     \
        .signal_SIGXCPU = true,     \
        .signal_SIGXFSZ = true,     \
                                    \
        .signal_SIGPIPE = true,     \
        .signal_SIGPOLL = true,     \
        .signal_SIGURG = true,      \
                                    \
        .signal_SIGALRM = true,     \
        .signal_SIGVTALRM = true,   \
        .signal_SIGPROF = true,     \
                                    \
        .signal_SIGHUP = true,      \
        .signal_SIGTTIN = true,     \
        .signal_SIGTTOU = true,     \
        .signal_SIGWINCH = true,    \
                                    \
        .signal_SIGUSR1 = true,     \
        .signal_SIGUSR2 = true,     \
    }

/**
 * @brief Signal handler function declarator.
 *
 * @see station_signal_handler_func_t
 */
#define STATION_SIGNAL_HANDLER_FUNC(name)               \
    bool name(int signo, void *siginfo,                 \
            struct station_std_signal_set *std_signals, \
            struct station_rt_signal_set *rt_signals,   \
            void *data)

#endif // _STATION_SIGNAL_DEF_H_

