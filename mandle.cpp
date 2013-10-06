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

/** Our main header */
#include "mandle.h"

/**
 * The strategy name, used for the CSV and the window name in case of a X11 enabled build 
 */
const char* get_strategy_name(int strategy) {
    if ( strategy == STRATEGY_STATIC )
        return "MPI-Static";
    else if ( strategy == STRATEGY_STATIC_RR )
        return "MPI-Static-RoundRobin";
    return "MPI-Dynamic"; 
}

/**
 * Main entry point
 */
int main (int argc, char *argv[]) {

    int nProcs;
    int myID;
    int returnval;
    int iterations;
    double real_min = -SIZE;
    double real_max = SIZE;
    double imag_min = -SIZE;
    double imag_max = SIZE;
    int width = X_PIX;
    int height = Y_PIX;
    int strategy = STRATEGY_STATIC;

    // Initialize and check for commands
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        ERROR("MPI initialization error\n");
        exit(EXIT_FAILURE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myID);
    if (nProcs < 2) {
        if (myID == 0) {
            ERROR("Number of processes must be at least 2\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // Sanity checks
    if ( argc < 2 ) {
        if (myID == 0) {
            ERROR("Usage: %s iterations [strategy sizeY sizeY] %d\n", argv[0], argc);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // Get data from commandline
    iterations = atoi(argv[1]);
    if (argc > 2)
        strategy = atoi(argv[2]);
    if (argc > 4) {
        width = atof(argv[3]);
        height = atof(argv[4]);
    }

    // Make sure we got a valid strategy
    if ( strategy != STRATEGY_STATIC && strategy != STRATEGY_STATIC_RR && strategy != STRATEGY_DYNAMIC ) {
        if (myID == 0) {
            ERROR("Strategy '%d' not valid\n", strategy);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // Now call a master or a slave process
    if (myID == 0) {
#if WITH_X11
        initX11(get_strategy_name(strategy), width, height, 0, 0);
#endif
        master_proc(strategy, nProcs-1, width, height, real_min, real_max, imag_min, imag_max, iterations);
#if WITH_X11
        flushX11AndWait(30);
#endif
    }
    else {
        worker_proc(strategy, myID, nProcs-1, width, height, real_min, real_max, imag_min, imag_max, iterations);
    }

    // We are donw :D
    MPI_Finalize();
    return EXIT_SUCCESS;
}

/**
 * The master process, will distribute the work to the worker processes and wait for them to finish.
 */
void master_proc(int strategy, int num_processes, int width, int height, double real_min, double real_max, double imag_min, double imag_max, int iters) {
    LOG("Master Process\n");
    // Basic values for our process, this is used byt both version,
    // the static and the round-robin version.
    long color_min = 0;
    long color_max = 0;
    long initial_msg[MSG_FROM_MASTER_LEN];
    int initial_row, cur_row, next_row;
    int num_rows, rows_per_worker, rows_per_worker_left;
    int id, workers_active;
    MPI_Status mpi_status;

    long* recv_msg = (long*)malloc((width+1) * sizeof(*recv_msg));

    // The following vars are used for timing stuff
    double start_time, end_time;

    // Send the start color values
    MPI_Bcast(&color_max, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&color_min, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    // Start
    start_time = MPI_Wtime();

    if ( strategy == STRATEGY_STATIC ) {
        // Calculate the number of roww per worker and send them the
        // required data to start (start row and number of rows)
        initial_row = 0;
        rows_per_worker = height / num_processes;
        rows_per_worker_left = height % num_processes;

        // Send work to worker processed
        for (int process = 0; process < num_processes; ++process) {
            // Accum row number
            if ( process < rows_per_worker_left )
                num_rows = rows_per_worker + 1;
            else
                num_rows = rows_per_worker;

            // Send to the prcess the start row and the number of rows to be processed
            initial_msg[0] = initial_row;
            initial_msg[1] = num_rows;
            MPI_Send(initial_msg, MSG_FROM_MASTER_LEN, MPI_LONG, process+1, MSG_FROM_MASTER, MPI_COMM_WORLD);

            // Shift initial_rows by num_rows to prepare next process
            initial_row += num_rows;
        }
    } else if ( strategy == STRATEGY_DYNAMIC ) {
        // Send each worker a starting row
        next_row = 0;
        workers_active = 0;
        for (int process = 0; process < num_processes; ++process) {
            MPI_Send(&next_row, 1, MPI_INT, process+1, MSG_FROM_MASTER_WORK, MPI_COMM_WORLD);
            ++next_row;
            ++workers_active;
        }
    }

#if WITH_PBM
    // Allocate enough for the final image
    char* mandleData = (char*) calloc(width * height, sizeof(char));
#endif

    if ( strategy == STRATEGY_STATIC || strategy == STRATEGY_STATIC_RR ) {
        // Wait for work to be completed
        for (int row = 0; row < height; ++row) {
            MPI_Recv(recv_msg, width+1, MPI_LONG, MPI_ANY_SOURCE, MSG_FROM_WORKER, MPI_COMM_WORLD, &mpi_status);
#if WITH_PBM || WITH_X11
            cur_row = recv_msg[0];
            for (int col = 0; col < width; ++col) {
#if WITH_PBM
                mandleData[(cur_row*height)+col] = 0;
#endif
                if ( recv_msg[col+1] == 1 ) {
#if WITH_PBM
                    mandleData[(cur_row*height)+col] = 1;
#endif
#if WITH_X11
                    drawPoint(col, cur_row);
#endif
                }
            }
#endif
        }
    } else if ( strategy == STRATEGY_DYNAMIC ) {
        // If we got workers active go on!
        while (workers_active > 0) {
            MPI_Recv(recv_msg, width+1, MPI_LONG, MPI_ANY_SOURCE, MSG_FROM_WORKER, MPI_COMM_WORLD, &mpi_status);

            --workers_active;
            id = mpi_status.MPI_SOURCE;

            // Check for work left
            if (next_row < height) {
                MPI_Send(&next_row, 1, MPI_INT, id, MSG_FROM_MASTER_WORK, MPI_COMM_WORLD);
                ++next_row;
                ++workers_active;
            } else {
                MPI_Send(&next_row, 0, MPI_INT, id, MSG_FROM_MASTER_STOP, MPI_COMM_WORLD);
            }

#if WITH_PBM || WITH_X11
            // Draw what we have
            cur_row = recv_msg[0];
            for (int col = 0; col < width; ++col) {
#if WITH_PBM
                mandleData[(cur_row*height)+col] = 0;
#endif
                if ( recv_msg[col+1] == 1 ) {
#if WITH_PBM
                    mandleData[(cur_row*height)+col] = 1;
#endif
#if WITH_X11
                    drawPoint(col, cur_row);
#endif
                }
            }
#endif
        }
    }

    // Finished
    end_time = MPI_Wtime();

    // Create file
    FILE* output;
    output = fopen ( "output.csv" , "a+" );
#if WITH_OMP
    fprintf(output, "%s,%d,%d,%d,%g\n", get_strategy_name(strategy), num_processes, omp_get_max_threads(), (width*height), end_time - start_time);
#else
    fprintf(output, "%s,%d,%d,%g\n", get_strategy_name(strategy), num_processes, (width*height), end_time - start_time);
#endif
    fclose (output);

#if WITH_PBM
    // Create PBM file
    createPBMFile("out.pbm", mandleData, width, height);
    free(mandleData);
#endif

    free(recv_msg);
}

/**
 * The worker process, will process those rows that the master told him and send the result back.
 */
void worker_proc(int strategy, int ID, int num_processes, int width, int height, double real_min, double real_max, double imag_min, double imag_max, int iters) {
    LOG("Worker: %d\n", ID);

    // Basic values for our process, this is used byt both version,
    // the static and the round-robin version.
    long color_min; // No assigmnent needed, will come from the master
    long color_max; // No assigmnent needed, will come from the master
    double scale_real, scale_imag;
    long initial_msg[MSG_FROM_MASTER_LEN];
    int initial_row, num_rows, last_row, cur_row;
    MPI_Status mpi_status;

    long* send_msg = (long*)malloc((width+1) * sizeof(*send_msg));

    // Get color values from the master process
    MPI_Bcast(&color_max, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&color_min, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    // Compute scale factors
    scale_real = (double) (real_max - real_min) / (double) width;
    scale_imag = (double) (imag_max - imag_min) / (double) height; 

    if ( strategy == STRATEGY_STATIC ) {
        // Get the job data from the master
        MPI_Recv(initial_msg, MSG_FROM_MASTER_LEN, MPI_LONG, 0, MSG_FROM_MASTER, MPI_COMM_WORLD, &mpi_status);
        initial_row = initial_msg[0];
        num_rows = initial_msg[1];
        last_row = initial_row + num_rows;

        for (int i = initial_row; i < last_row; ++i) {
            computeMandleColum(send_msg, width, i, scale_real, scale_imag, iters, height, real_min, imag_min); 
            MPI_Send(send_msg, width+1, MPI_LONG, 0, MSG_FROM_WORKER, MPI_COMM_WORLD);
        }
    } else if ( strategy == STRATEGY_STATIC_RR ) {
        for (int i = (ID-1); i < height; i += num_processes) {
            computeMandleColum(send_msg, width, i, scale_real, scale_imag, iters, height, real_min, imag_min);
            MPI_Send(send_msg, width+1, MPI_LONG, 0, MSG_FROM_WORKER, MPI_COMM_WORLD);
        }
    } else if ( strategy == STRATEGY_DYNAMIC ) {
        // Work until we have no more work to be done
        while ( ((MPI_Recv(&cur_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status)) == MPI_SUCCESS) && (mpi_status.MPI_TAG == MSG_FROM_MASTER_WORK) ) {
            computeMandleColum(send_msg, width, cur_row, scale_real, scale_imag, iters, height, real_min, imag_min);
            MPI_Send(send_msg, width+1, MPI_LONG, 0, MSG_FROM_WORKER, MPI_COMM_WORLD);
        }
    }

    LOG("Worker: %d - Finished\n", ID);

    free(send_msg);
}
