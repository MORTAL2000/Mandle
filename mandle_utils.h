/**
 * Mandlebort implementation over MPI using static job assignment
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

#ifndef MANDLE_UTILS_H
#define MANDLE_UTILS_H

/** STD includes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
using namespace std;

/** MPI bug because it will redefine those and confict with stdlib */
// #undef SEEK_SET
// #undef SEEK_END
// #undef SEEK_CUR

#ifndef WITH_MPI
	#define WITH_MPI 1
#endif

#if WITH_MPI
	#include <mpi.h>
#endif

#ifndef WITH_OMP
	#define WITH_OMP 0
#endif

#if WITH_OMP
	#include <omp.h>
	
	// Set the right mode for OMP
	#ifndef SET_OMP_MODE
		#define OMP_MODE static
	#else
		#if SET_OMP_MODE == 1
			#define OMP_MODE dynamic
		#elif SET_OMP_MODE == 2
			#define OMP_MODE guided
		#else
			#define OMP_MODE static
		#endif
	#endif
#endif

#ifndef WITH_BENCHMARK
	#define WITH_BENCHMARK 0
#endif

// Define usage of X11 only if not benchmarking
#if WITH_BENCHMARK
	#undef WITH_X11
	#define WITH_X11 0
#else
	#ifndef WITH_X11
		#define WITH_X11 0
	#endif
#endif

#if WITH_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#endif

// Define usage of PBM only if not benchmarking
#if WITH_BENCHMARK
	#undef WITH_PBM
	#define WITH_PBM 0
#else
	#ifndef WITH_PBM
		#define WITH_PBM 0
	#endif
#endif

// Logging
#ifdef DEBUG
	#define LOG(args...) fprintf(stdout, args);
#else
 	#define LOG(args...)
#endif
#define ERROR(args...) fprintf(stderr, args);

// Simple output methos
#define PRINT(args...)  fprintf(stdout, args);

/** x and y pixel size */
#define	X_PIX	800
#define	Y_PIX	800

/** Problems size, x and y go from -SIZE to SIZE */
#define SIZE	2
#define SIZE_SQ	SIZE*SIZE

/* Complex umber definition */
typedef struct {
    double real;
    double imag;
} COMPLEX;

/** Get current time */
double GetTime();

#if WITH_PBM
/**
 * Generate a plain PBM file.
 *
 * filename: filename to be written to
 * data: matrix of all data
 */
void createPBMFile(const char* filename, char *data, int width, int height);
#endif

/**
 * Compute the mandlebrot set and return 0 or 1 for a given location
 */
char computeMandle(int row, int column, double scale_real, double scale_imag, int iters, int height, double real_min, double imag_min);

/**
 * Compute the mandlebrot set for a given location and store the data in a pre allocated array
 */
void computeMandleColum(long *data, int width, int row, double scale_real, double scale_imag, int iters, int height, double real_min, double imag_min);

#if WITH_X11

extern Window      win;
extern GC          gc;
extern Display*    display;

/**
 * Initialize the X11 display
 */
void initX11(const char* window_title, unsigned int width, unsigned int height, int x, int y);

/**
 * Draw a black point at a given position
 */
void drawPoint(int x, int y);

/**
 * Flush X11 display and sleep for some seconds
 */
void flushX11AndWait(int seconds);
#endif

#endif // MANDLE_UTILS_H