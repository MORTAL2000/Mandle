/**
 * Mandlebort implementation using OpenCL
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
#include "mandle_cl.h"

cl_mem AllocPixelBuffer(cl_context context, const size_t buffer_size, cl_int* errorn) {
    return clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, errorn);
}

void FreePixelBuffer(cl_mem pixelBuffer) {
    const cl_int errorn = clReleaseMemObject(pixelBuffer);
    clu_check_error("FreePixelBuffer", errorn);
}

/**
 * Main entry point
 */
int main (int argc, char *argv[]) {
	int iterations;
	int width = X_PIX;
	int height = Y_PIX;
    float scale = 3.5f;
    float offsetX = -0.5f;
    float offsetY = 0;

    // Sanity checks
    if ( argc < 2 ) {
        ERROR("Usage: %s iterations [sizeY sizeY scale offsetX offestY] %d\n", argv[0], argc);
        exit(EXIT_FAILURE);
    }

    // Get data from commandline
    iterations = atoi(argv[1]);
    if (argc > 3) {
         width = atof(argv[2]);
         height = atof(argv[3]);
    }
    if (argc > 4) {
        scale = atof(argv[4]);
    }
    if (argc > 5) {
        offsetX = atof(argv[5]);
        offsetY = atof(argv[6]);
    }

    // List of OpenCL specifc variables
    cl_kernel kern;
    cl_command_queue queue;
    cl_context context;
    cl_int errorn = 0;
    cl_device_id* devices = NULL;
    unsigned int workGroupSize = 1;

    // Create the context
    context = clu_create_context(CL_DEVICE_TYPE_ALL);

    // Get devices
    devices = clu_get_devices(context);

    // Load kernel
    kern = clu_load_kernel(context, "mandel_kernel.cl", "mandel_kernel", devices);

    // Create our work group
    queue = clu_create_command_queue(context, kern, devices, 0,&workGroupSize);

    if(devices == NULL) {
        ERROR("No valid OpenCL devices found. Sorry...\n");
        return EXIT_FAILURE;
    }

    size_t mandleData_size = sizeof(char) * width * height;
    cl_mem pixelBuffer = AllocPixelBuffer(context, mandleData_size, &errorn);
    clu_check_error("Creating pixel buffer", errorn);

    // Set kernel arguments
    errorn = clSetKernelArg(
            kern,
            0,
            sizeof(cl_mem),
            (void *) &pixelBuffer);
    clu_check_error("setup_arguments mandleData", errorn);

    errorn = clSetKernelArg(
            kern,
            1,
            sizeof(int),
            (void *)&width);
    clu_check_error("setup_arguments width", errorn);

    errorn = clSetKernelArg(
            kern,
            2,
            sizeof(int),
            (void *)&height);
    clu_check_error("setup_arguments height", errorn);

    errorn = clSetKernelArg(
            kern,
            3,
            sizeof(float),
            (void *)&scale);
    clu_check_error("setup_arguments scale", errorn);

    errorn = clSetKernelArg(
            kern,
            4,
            sizeof(float),
            (void *)&offsetX);
    clu_check_error("setup_arguments offsetX", errorn);

    errorn = clSetKernelArg(
            kern,
            5,
            sizeof(float),
            (void *)&offsetY);
    clu_check_error("setup_arguments offsetY", errorn);

    errorn = clSetKernelArg(
            kern,
            6,
            sizeof(int),
            (void *)&iterations);
    clu_check_error("setup_arguments iters", errorn);

    // Start timer
    double start = GetTime();

    // Enqueue a kernel run call
    cl_event events[2];
    size_t globalThreads[1];
    globalThreads[0] = width * height + 1;
    if (globalThreads[0] % workGroupSize != 0) {
        globalThreads[0] = (globalThreads[0] / workGroupSize + 1) * workGroupSize;
    }
    size_t localThreads[1];
    localThreads[0] = workGroupSize;

    errorn = clEnqueueNDRangeKernel(
            queue,
            kern,
            1,
            NULL,
            globalThreads,
            localThreads,
            0,
            NULL,
            &events[0]);
    clu_check_error("Failed to push queue", errorn);

    // Wait for the kernel call to finish execution
    errorn = clWaitForEvents(1, &events[0]);
    clu_check_error("CFailed to wait for work to be finished", errorn);

    clReleaseEvent(events[0]);

    // Allocate the char buffer used to draw the mandlebrot into
    char* mandleData = (char*) calloc(width * height, sizeof(char));

    // Enqueue readBuffer
    errorn = clEnqueueReadBuffer(
            queue,
            pixelBuffer,
            CL_TRUE,
            0,
            width * height,
            mandleData,
            0,
            NULL,
            &events[1]);
    clu_check_error("Failed to read computation result", errorn);

    clReleaseEvent(events[1]);

    double end = GetTime();
    const double elapsedTime = end - start;
    const double sampleSec = elapsedTime>0?height * width / elapsedTime:0;

    // Create file
    FILE* output;
    output = fopen ( "output_cl.csv" , "a+" );
    fprintf(output, "%g,%g,%d\n", elapsedTime, sampleSec / 1000.f,(width*height));
    fclose (output);

    // Free pixel buffer
    FreePixelBuffer(pixelBuffer);

    // Write PBM file
#if WITH_PBM
    createPBMFile("out_cl.pbm", mandleData, width, height);
#endif
    free(mandleData);
    
    return EXIT_SUCCESS;
}