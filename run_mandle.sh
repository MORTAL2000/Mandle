#!/bin/sh
# Just run the mpi static example
date
echo "process starting"
mpirun -np $1 ./bin/mandle.o $2 $3 $4 $5 $6