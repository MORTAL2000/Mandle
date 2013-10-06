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

/** Our main header */
#include "mandle_cl_utils.h"
#include "mandle_utils.h"

/**
 * Read a file and load into a char buffer
 */
const char* clu_read_file(const char *filename) {
    // return fdata;
    FILE *file = fopen(filename, "r");
    if (!file) {
        ERROR("Failed to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END)) {
        ERROR("Failed to seek file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    long size = ftell(file);
    if (size == 0) {
        ERROR("Failed to check position on file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    rewind(file);

    char *src = (char *)malloc(sizeof(char) * size + 1);
    if (!src) {
        ERROR("Failed to allocate memory for file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    LOG("Reading file '%s' (size %ld bytes)\n", filename, size);
    size_t res = fread(src, 1, sizeof(char) * size, file);
    if (res != sizeof(char) * size) {
        ERROR("Failed to read file '%s' (read %ld)\n", filename, res);
        exit(EXIT_FAILURE);
    }
    src[size] = '\0'; // NULL terminated

    fclose(file);

    return src;
}

/**
 * Load a kernel programm
 */
cl_kernel clu_load_kernel(cl_context context, const char *filename, const char *kernelname, cl_device_id *devices) {
    cl_int errorn;
    // Create the kernel program
    const char *sources = clu_read_file(filename);
    cl_program program = clCreateProgramWithSource(
            context,
            1,
            &sources,
            NULL,
            &errorn);
    clu_check_error("clu_load_kernel-clCreateProgramWithSource", errorn);

    errorn = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if (errorn != CL_SUCCESS) {
        clu_check_error("Failed to build kernel", errorn, false);

        size_t retValSize;
        errorn = clGetProgramBuildInfo(
                program,
                devices[0],
                CL_PROGRAM_BUILD_LOG,
                0,
                NULL,
                &retValSize);
        clu_check_error("Failed to get kernel info", errorn);

        char *buildLog = (char *)malloc(retValSize + 1);
        errorn = clGetProgramBuildInfo(
                program,
                devices[0],
                CL_PROGRAM_BUILD_LOG,
                retValSize,
                buildLog,
                NULL);
        clu_check_error("Failed to get kernel build log", errorn);
        buildLog[retValSize] = '\0';

        ERROR("OpenCL Programm Build Log:\n%s\n", buildLog);
        exit(EXIT_FAILURE);
    }

    cl_kernel kernel = clCreateKernel(program, kernelname, &errorn);
    clu_check_error("Failed to create kernel", errorn);
    return kernel;
}

/**
 * Create a OpenCL context. Will try to use GPU but is able to fallback
 * to a CPU context in case no GPU is present.
 */
cl_context clu_create_context(cl_device_type type) {
    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    cl_int errorn = clGetPlatformIDs(0, NULL, &numPlatforms);
    clu_check_error("Failed to get platforms", errorn);

    if (numPlatforms > 0) {
        cl_platform_id *platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
        errorn = clGetPlatformIDs(numPlatforms, platforms, NULL);
        clu_check_error("Failed to get platform ids", errorn);

        unsigned int i;
        for (i = 0; i < numPlatforms; ++i) {
            char pbuf[100];
            errorn = clGetPlatformInfo(platforms[i],
                    CL_PLATFORM_VENDOR,
                    sizeof(pbuf),
                    pbuf,
                    NULL);

            errorn = clGetPlatformIDs(numPlatforms, platforms, NULL);
            clu_check_error("Failed to get platform ids", errorn);

            LOG("OpenCL Platform %d: %s\n", i, pbuf);
        }

        platform = platforms[0];
        free(platforms);
    }

    cl_context_properties cps[3] ={
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) platform,
        0
    };

    cl_context_properties *cprops = (NULL == platform) ? NULL : cps;

    cl_context context = clCreateContextFromType(
            cprops,
            type,
            NULL,
            NULL,
            &errorn);
    clu_check_error("Failed to OpenCL context", errorn);

    return context;
}

/**
 * Get the devices available by the context
 */
cl_device_id* clu_get_devices(cl_context context) {
    // Get the size of device list data
    size_t deviceListSize;
    cl_int errorn = clGetContextInfo(
            context,
            CL_CONTEXT_DEVICES,
            0,
            NULL,
            &deviceListSize);
    clu_check_error("Failed to OpenCL context device size", errorn);

    cl_device_id* devices = (cl_device_id*) malloc(deviceListSize);
    if (devices == NULL) {
        ERROR("Failed to allocate devices\n");
        exit(EXIT_FAILURE);
    }

    // Get the device list data
    errorn = clGetContextInfo(
            context,
            CL_CONTEXT_DEVICES,
            deviceListSize,
            devices,
            NULL);
    clu_check_error("Failed to OpenCL context device list", errorn);
    return devices;
}

/**
 * Create a command queue
 */
cl_command_queue clu_create_command_queue(cl_context context, cl_kernel kernel, cl_device_id *devices, const int device, unsigned int* wg_size) {
    size_t gsize = 0;
    cl_int errorn = clGetKernelWorkGroupInfo(kernel, devices[device], CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &gsize, NULL);
    clu_check_error("Creating work group", errorn);

    *wg_size = (unsigned int) gsize;
    LOG("OpenCL Device %d: kernel work group size = %d\n", device, *wg_size);

    cl_command_queue_properties prop = 0;
    return clCreateCommandQueue(context, devices[device], prop, &errorn);
}

/**
 * Check for an OpenCL error
 */
void clu_check_error(const char* msg, cl_int errorn, bool bExitOnError/*=true*/) {
    if ( errorn != CL_SUCCESS ) {
        ERROR("[%s] OpenCL Error: ", msg);
        switch(errorn) {
            case -1: ERROR("Device not found\n"); break;
            case -2: ERROR("Device not available\n"); break;
            case -3: ERROR("Compiler not available\n"); break;
            case -4: ERROR("Memory object allocation failure\n"); break;
            case -5: ERROR("Out of resources\n"); break;
            case -6: ERROR("Out of host memory\n"); break;
            case -7: ERROR("Profiling info not available\n"); break;
            case -8: ERROR("Memory copy overlap\n"); break;
            case -9: ERROR("Image format mismatch\n"); break;
            case -10: ERROR("Image format not supported\n"); break;
            case -11: ERROR("Build program failure\n"); break;
            case -12: ERROR("Map failure\n"); break;
            case -30: ERROR("Invalid value\n"); break;
            case -31: ERROR("Invaid device type\n"); break;
            case -32: ERROR("Invalid platform\n"); break;
            case -33: ERROR("Invalid device\n"); break;
            case -34: ERROR("Invalid context\n"); break;
            case -35: ERROR("Invalid queue properties\n"); break;
            case -36: ERROR("Invalid command queue\n"); break;
            case -37: ERROR("Invalid host pointer\n"); break;
            case -38: ERROR("Invalid memory object\n"); break;
            case -39: ERROR("Invalid image format descriptor\n"); break;
            case -40: ERROR("Invalid image size\n"); break;
            case -41: ERROR("Invalid sampler\n"); break;
            case -42: ERROR("Invalid binary\n"); break;
            case -43: ERROR("Invalid build options\n"); break;
            case -44: ERROR("Invalid program\n"); break;
            case -45: ERROR("Invalid program executable\n"); break;
            case -46: ERROR("Invalid kernel name\n"); break;
            case -47: ERROR("Invalid kernel defintion\n"); break;
            case -48: ERROR("Invalid kernel\n"); break;
            case -49: ERROR("Invalid argument index\n"); break;
            case -50: ERROR("Invalid argument value\n"); break;
            case -51: ERROR("Invalid argument size\n"); break;
            case -52: ERROR("Invalid kernel arguments\n"); break;
            case -53: ERROR("Invalid work dimension\n"); break;
            case -54: ERROR("Invalid work group size\n"); break;
            case -55: ERROR("Invalid work item size\n"); break;
            case -56: ERROR("Invalid global offset\n"); break;
            case -57: ERROR("Invalid event wait list\n"); break;
            case -58: ERROR("Invalid event\n"); break;
            case -59: ERROR("Invalid operation\n"); break;
            case -60: ERROR("Invalid GL object\n"); break;
            case -61: ERROR("Invalid buffer size\n"); break;
            case -62: ERROR("Invalid mip level\n"); break;
            case -63: ERROR("Invalid global work size\n"); break;
        }
        if ( bExitOnError )
            exit(EXIT_FAILURE);
    }
}