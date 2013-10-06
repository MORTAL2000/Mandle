/**
 * OpenCL Utility library (clu)
 *
 * Copyright (c) 2012, Moritz Wundke
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the owner nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Moritz Wundke BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef MANDLE_CL_UTILS_H
#define MANDLE_CL_UTILS_H

/** OpenCL headers */
#include <CL/opencl.h>
#include <CL/cl.h>

/** Maximum number of devices we are able to handle */
#define MAX_DEVICES 16

/**
 * Read a file and load into a char buffer
 */
const char* clu_read_file(const char *filename);

/**
 * Load a kernel programm
 */
cl_kernel clu_load_kernel(cl_context context, const char *filename, const char *kernelname, cl_device_id *devices);

/**
 * Create a context and provide a list of available devices
 */
cl_context clu_create_context(cl_device_type type);

/**
 * Get the devices available by the context
 */
cl_device_id* clu_get_devices(cl_context context);

/**
 * Create a command queue
 */
cl_command_queue clu_create_command_queue(cl_context context, cl_kernel kernel, cl_device_id *devices, const int device, unsigned int* wg_size);

/**
 * Check for an OpenCL error
 */
void clu_check_error(const char* msg, cl_int err, bool bExitOnError=true);

#endif // MANDLE_CL_UTILS_H