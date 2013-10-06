/**
 * Mandlebort implementation over MPI and OpenMP
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

/** Our own includes */
#include "mandle_utils.h"

/** Get current time */
double GetTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

#if WITH_PBM   
/**
 * Generate a plain PBM file.
 *
 * filename: filename to be written to
 * data: matrix of all data
 */
void createPBMFile(const char* filename, char *data, int width, int height)
{
	// Create file
	FILE* pbmFile;
	pbmFile = fopen ( filename , "w" );

	// Write data to the PBM file
	int i, j;
	fprintf(pbmFile, "P1\n");
	fprintf(pbmFile, "%d %d\n", width, height); 
	for (i = 0; i < height; i++) 
	{
		for (j = 0; j < width; j++) {
			fprintf(pbmFile, "%d", data[(i*height)+j]);
		}
		fprintf(pbmFile, "\n"); 
	}

	// Close file
	fclose (pbmFile);
}
#endif

/**
 * Compute the mandlebrot set and return 0 or 1 for a given location
 */
char computeMandle(int row, int column, double scale_real, double scale_imag, int iters, int height, double real_min, double imag_min) {
    COMPLEX z, c;
    z.real = z.imag = 0;

    // Do the mandlebort magic
    c.real = real_min + ((double) column * scale_real);
    c.imag = imag_min + ((double) (height-1-row) * scale_imag);
    int k = 0;
    double lengthsq, temp;
    do  {
        temp = z.real*z.real - z.imag*z.imag + c.real;
        z.imag = 2.0*z.real*z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real*z.real + z.imag*z.imag;
        ++k;
    } while (lengthsq < SIZE_SQ && k < iters);
    if (k == iters) {
        return 1;
    }
    return 0;
}

/**
 * Compute the mandlebrot set for a given location and store the data in a pre allocated array
 */
void computeMandleColum(long *data, int width, int row, double scale_real, double scale_imag, int iters, int height, double real_min, double imag_min) {
	// Set the row id for the data set
    data[0] = row;

    // Get the color data for each column
    int j;
#if WITH_OMP
    int tid;
    #pragma omp parallel shared(data,width,row,scale_real,scale_imag,iters,height,real_min,imag_min) private(j,tid)
#endif
    {
#if WITH_OMP
        tid = omp_get_thread_num();
#ifdef OMP_CHUNK
        #pragma omp parallel for schedule(OMP_MODE, OMP_CHUNK)
#else
        #pragma omp parallel for schedule(OMP_MODE)
#endif
#endif
        for (j = 0; j < width; ++j) {
            data[j+1] = computeMandle(row, j, scale_real, scale_imag, iters, height, real_min, imag_min);
#if WITH_OMP
            LOG("Thread %d: row:%d col:%d)\n",tid,row,j);
#endif
        }
    }
}

// X11 values
#if WITH_X11

Window      win;
GC          gc;
Display*    display;

/**
 * Initialize the X11 display
 */
void initX11(const char*  window_title, unsigned int width, unsigned int height, int x, int y) {
	unsigned int window_width, window_height,                  /* window size */
            border_width,                   /* border width in pixels */
			display_width, display_height,  /* size of screen */
            screen;                         /* which screen */

    const char            *window_name = window_title, *display_name = NULL;
    unsigned
    long        valuemask = 0;
    XGCValues   values;
    XSizeHints  size_hints;
    Pixmap      bitmap;
    XPoint      points[800];
    FILE        *fp, *fopen ();
    char        str[100];

    XSetWindowAttributes attr[1];

    /* connect to Xserver */

    if (  (display = XOpenDisplay (display_name)) == NULL ) {
        ERROR ("XOpenDisplay: cannot connect to X server %s. Process will contiue.\n", XDisplayName (display_name) );
    }

    /* get screen size */

    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* create opaque window */

    border_width = 4;
    win = XCreateSimpleWindow (display, RootWindow (display, screen),
    x, y, width, height, border_width, 
    BlackPixel (display, screen), WhitePixel (display, screen));

    size_hints.flags = USPosition|USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;

    XSetNormalHints (display, win, &size_hints);
    XStoreName(display, win, window_name);

    /* create graphics context */

    gc = XCreateGC (display, win, valuemask, &values);

    XSetBackground (display, gc, WhitePixel (display, screen));
    XSetForeground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel(display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);
}

/**
 * Draw a black point at a given position
 */
void drawPoint(int x, int y) {
	XDrawPoint (display, win, gc, x, y);
}

/**
 * Flush X11 display and sleep for some seconds
 */
void flushX11AndWait(int seconds) {
	XFlush (display);
    sleep (seconds);
}
#endif
