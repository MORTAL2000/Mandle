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
#ifndef MANDLE_H
#define MANDLE_H

/** STD includes */
#include <stdio.h>
#include <stdlib.h>
 
/** Our own includes */
#include "mandle_utils.h"

/** Message id's used to send to the workers and what the workers send the master */
#define MSG_FROM_MASTER 		1
#define MSG_FROM_WORKER			2
#define MSG_FROM_MASTER_WORK	3
#define MSG_FROM_MASTER_STOP    4

/** Message lengths */
#define MSG_FROM_MASTER_LEN	2

#define STRATEGY_STATIC		0
#define STRATEGY_STATIC_RR	1
#define STRATEGY_DYNAMIC	2

/**
 * The strategy name, used for the CSV and the window name in case of a X11 enabled build 
 */
const char* get_strategy_name(int strategy);

/**
 * The master process, will distribute the work to the worker processes and wait for them to finish.
 */
void master_proc(int strategy, int num_processes, int width, int height, double real_min, double real_max, double imag_min, double imag_max, int iters);

/**
 * The worker process, will process those rows that the master told him and send the result back.
 */
void worker_proc(int strategy, int ID, int num_processes, int width, int height, double real_min, double real_max, double imag_min, double imag_max, int iters);

#endif // MANDLE_H