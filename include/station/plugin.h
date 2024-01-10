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
 * @brief Minimum set of headers required for a user plugin.
 */

#pragma once
#ifndef _STATION_PLUGIN_H_
#define _STATION_PLUGIN_H_

#include <station/plugin.def.h> // for plugin implementation macros
#include <station/plugin.typ.h> // for plugin binary compatibility types
#include <station/index.typ.h>  // for index types
#include <station/state.typ.h>  // for finite state machine state type
#include <station/sdl.typ.h>    // for SDL context
#include <station/opencl.typ.h> // for OpenCL context

#endif // _STATION_PLUGIN_H_

