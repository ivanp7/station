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

#define STATION_APP_ERROR_BASE 64

#define STATION_APP_ERROR_ATEXIT        (STATION_APP_ERROR_BASE + 1)
#define STATION_APP_ERROR_ARGUMENTS     (STATION_APP_ERROR_BASE + 2)
#define STATION_APP_ERROR_PLUGIN        (STATION_APP_ERROR_BASE + 3)
#define STATION_APP_ERROR_MALLOC        (STATION_APP_ERROR_BASE + 4)
#define STATION_APP_ERROR_SIGNAL        (STATION_APP_ERROR_BASE + 5)
#define STATION_APP_ERROR_THREADS       (STATION_APP_ERROR_BASE + 6)
#define STATION_APP_ERROR_OPENCL        (STATION_APP_ERROR_BASE + 7)
#define STATION_APP_ERROR_SDL           (STATION_APP_ERROR_BASE + 8)

#endif // _STATION_APPLICATION_DEF_H_

