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
 * @brief Application entry point for standalone plugins.
 */

#pragma once
#ifndef _STATION_APPLICATION_FUN_H_
#define _STATION_APPLICATION_FUN_H_

struct station_plugin_format;
struct station_plugin_vtable;

/**
 * @brief Application entry point.
 *
 * @return Application exit code.
 */
int
station_app_main(
        int argc,     ///< [in] Number of command line arguments.
        char *argv[], ///< [in] Command line arguments.

        struct station_plugin_vtable *plugin_vtable  ///< [in] Plugin vtable.
);

#endif // _STATION_APPLICATION_FUN_H_

