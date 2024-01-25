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
 * @brief Types for signal management.
 */

#pragma once
#ifndef _STATION_SIGNAL_TYP_H_
#define _STATION_SIGNAL_TYP_H_

#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h>
#endif

#include <stdbool.h>

/**
 * @brief Set of supported standard signals.
 */
typedef struct station_std_signal_set {
#ifndef __STDC_NO_ATOMICS__
    atomic_bool signal_SIGINT;
    atomic_bool signal_SIGQUIT;
    atomic_bool signal_SIGTERM;

    atomic_bool signal_SIGCHLD;
    atomic_bool signal_SIGCONT;
    atomic_bool signal_SIGTSTP;
    atomic_bool signal_SIGXCPU;
    atomic_bool signal_SIGXFSZ;

    atomic_bool signal_SIGPIPE;
    atomic_bool signal_SIGPOLL;
    atomic_bool signal_SIGURG;

    atomic_bool signal_SIGALRM;
    atomic_bool signal_SIGVTALRM;
    atomic_bool signal_SIGPROF;

    atomic_bool signal_SIGHUP;
    atomic_bool signal_SIGTTIN;
    atomic_bool signal_SIGTTOU;
    atomic_bool signal_SIGWINCH;

    atomic_bool signal_SIGUSR1;
    atomic_bool signal_SIGUSR2;
#else
    int dummy;
#endif
} station_std_signal_set_t;

/**
 * @brief Set of real-time signals.
 */
typedef struct station_rt_signal_set {
#ifndef __STDC_NO_ATOMICS__
    atomic_bool *signal_SIGRTMIN; ///< Array of RT signal flags of size (SIGRTMAX-SIGRTMIN+1).
#else
    int dummy;
#endif
} station_rt_signal_set_t;

/**
 * @brief Signal handler.
 *
 * @warning Do not use exit() or quick_exit() in this function,
 * otherwise the behavior is undefined!
 *
 * @return Whether the corresponding signal flag should be set.
 */
typedef bool (*station_signal_handler_func_t)(
        int signo,     ///< [in] Signal number.
        void *siginfo, ///< [in] Pointer to siginfo_t data structure.

        station_std_signal_set_t *std_signals, ///< [in,out] States of standard signals.
        station_rt_signal_set_t *rt_signals,   ///< [in,out] States of real-time signals.
        void *data                             ///< [in,out] Handler data.
);

#endif // _STATION_SIGNAL_TYP_H_

