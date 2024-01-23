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
 * @brief Types for OpenCL support.
 */

#pragma once
#ifndef _STATION_OPENCL_TYP_H_
#define _STATION_OPENCL_TYP_H_

#ifdef STATION_IS_OPENCL_SUPPORTED
#  include <CL/cl.h>
#endif

/**
 * @brief Array of OpenCL contexts and platform/device IDs.
 */
typedef struct station_opencl_contexts_array {
#ifdef STATION_IS_OPENCL_SUPPORTED
    cl_context *contexts; ///< Array of OpenCL platform contexts.
    struct {
        cl_platform_id platform_id; ///< OpenCL platform identifier.
        cl_device_id *device_ids;   ///< Array of OpenCL device identifiers.
        cl_uint num_devices;        ///< Number of utilized OpenCL devices on the platform.
    } *platforms;
    cl_uint num_platforms; ///< Number of OpenCL platforms with initialized contexts.
#else
    unsigned int num_platforms; // must be zero
#endif
} station_opencl_contexts_array_t;

#endif // _STATION_OPENCL_TYP_H_

