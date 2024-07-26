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
 * @brief Application error codes.
 */

#pragma once
#ifndef _STATION_APPLICATION_DEF_H_
#define _STATION_APPLICATION_DEF_H_

/**
 * @brief Application error exit code base.
 */
#define STATION_APP_ERROR_BASE 64

#define STATION_APP_ERROR_ATEXIT        (STATION_APP_ERROR_BASE +  1) ///< Error: atexit() or at_quick_exit() failed.
#define STATION_APP_ERROR_ARGUMENTS     (STATION_APP_ERROR_BASE +  2) ///< Error: incorrect command line arguments.
#define STATION_APP_ERROR_PLUGIN        (STATION_APP_ERROR_BASE +  3) ///< Error: couldn't load plugin.
#define STATION_APP_ERROR_MALLOC        (STATION_APP_ERROR_BASE +  4) ///< Error: malloc() failed.
#define STATION_APP_ERROR_FILE          (STATION_APP_ERROR_BASE +  5) ///< Error: couldn't read file.
#define STATION_APP_ERROR_SHAREDMEM     (STATION_APP_ERROR_BASE +  6) ///< Error: couldn't attach shared memory segment.
#define STATION_APP_ERROR_LIBRARY       (STATION_APP_ERROR_BASE +  7) ///< Error: couldn't load shared library.
#define STATION_APP_ERROR_SIGNAL        (STATION_APP_ERROR_BASE +  8) ///< Error: couldn't configure signal management.
#define STATION_APP_ERROR_THREADS       (STATION_APP_ERROR_BASE +  9) ///< Error: couldn't create concurrent processing context.
#define STATION_APP_ERROR_OPENCL        (STATION_APP_ERROR_BASE + 10) ///< Error: couldn't create OpenCL context.
#define STATION_APP_ERROR_SDL           (STATION_APP_ERROR_BASE + 11) ///< Error: couldn't initialize SDL subsystems.

/**
 * @brief Maximum value of application error exit code.
 */
#define STATION_APP_ERROR_MAX STATION_APP_ERROR_SDL

/**
 * @brief Define standalone plugin entry point.
 */
#define STATION_APP_PLUGIN_MAIN()                   \
    int main(int argc, char *argv[]) {              \
        return station_app_main(argc, argv,         \
                &STATION_PLUGIN_VTABLE_OBJECT); }

#endif // _STATION_APPLICATION_DEF_H_

