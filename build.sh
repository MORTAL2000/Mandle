#!/bin/sh
# Build the MPI only and the hybrid version.

# Cleanup bin dir
rm -rf bin/*
mkdir -p bin

# Compiler Options:
# -----------------
#  -DDEBUG to enable debug output
#  -DWITH_OMP -fopenmp to enable OpenMP hybrid support
#  -DWITH_X11 -lX11 result will be drawn using a X11 window
#  -DWITH_PBM enable PBM creation after the mandlebrot set has been created
#  -DWITH_BENCHMARK if set no PBM files or X11 output will be generated. Use this for benchmarking.
#  -DSET_OMP_MODE set the OpenMP schedule mode. 0 for static, 1 for dynamic and 2 guided
#  -DOMP_CHUNK set the OpenMP chunk size. By default 1.

echo "Create MPI only binary"
mpicxx -g mandle.cpp mandle_utils.cpp -o bin/mandle.o -DWITH_PBM -DWITH_BENCHMARK

echo "Create MPI-OpenMP hybrid binary (static)"
mpicxx -g mandle.cpp mandle_utils.cpp -o bin/mandle_hybrid_static.o -DWITH_OMP -fopenmp -DWITH_PBM=1 -DWITH_BENCHMARK -DSET_OMP_MODE=0

echo "Create MPI-OpenMP hybrid binary (dynamic)"
mpicxx -g mandle.cpp mandle_utils.cpp -o bin/mandle_hybrid_dynamic.o -DWITH_OMP -fopenmp -DWITH_PBM=1 -DWITH_BENCHMARK -DSET_OMP_MODE=1

echo "Create MPI-OpenMP hybrid binary (guided)"
mpicxx -g mandle.cpp mandle_utils.cpp -o bin/mandle_hybrid_guided.o -DWITH_OMP -fopenmp -DWITH_PBM=1 -DWITH_BENCHMARK -DSET_OMP_MODE=2

# Build OpenCL mandle sample. Change the location of your local AMD SDK installation
echo "Create OpenCL"
AMD_SDK=/opt/AMDAPP
export LD_LIBRARY_PATH=$AMD_SDK/lib/x86_64/
gcc -O3 -msse2 -mfpmath=sse -ftree-vectorize -funroll-loops -Wall -I $AMD_SDK/include -L $AMD_SDK/lib/x86_64 -DWITH_MPI=0 -DWITH_PBM=1 \
	mandle_cl.cpp mandle_utils.cpp mandle_cl_utils.cpp -o bin/mandle_cl.o -lOpenCL