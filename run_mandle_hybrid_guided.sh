#!/bin/sh
# Run the hybrid version
date
echo "process starting"
export OMP_NUM_THREADS=$1
mpirun -np $2 ./bin/mandle_hybrid_guided.o $3 $4 $5 $6 $7