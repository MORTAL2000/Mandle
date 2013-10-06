#!/bin/sh

OUTPUT=output
rm -rf $OUTPUT/*
mkdir -p $OUTPUT

DIMENSION=10000

# Create the CSV header
echo "Strategy,Workers,Dimension (width x height),Time" > output.csv

echo "Starting static strategy"
./run_mandle.sh 8 100 0 $DIMENSION $DIMENSION
./run_mandle.sh 16 100 0 $DIMENSION $DIMENSION

echo "Starting static strategy with round-robin"
./run_mandle.sh 8 100 1 $DIMENSION $DIMENSION
./run_mandle.sh 16 100 1 $DIMENSION $DIMENSION

echo "Starting dynamic strategy"
./run_mandle.sh 8 100 2 $DIMENSION $DIMENSION
./run_mandle.sh 16 100 2 $DIMENSION $DIMENSION

# Copy MPI csv to a different location
cp output.csv $OUTPUT/output_mpi.csv

# Create the CSV header
echo "Strategy,Workers,Threads,Dimension (width x height),Time" > output.csv

echo "Hybrid (static) - Starting static strategy"
./run_mandle_hybrid_static.sh 8 8 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 8 100 0 $DIMENSION $DIMENSION

./run_mandle_hybrid_static.sh 8 16 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 16 100 0 $DIMENSION $DIMENSION

echo "Hybrid (static) - Starting static strategy with round-robin"
./run_mandle_hybrid_static.sh 8 8 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 8 100 1 $DIMENSION $DIMENSION

./run_mandle_hybrid_static.sh 8 16 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 16 100 1 $DIMENSION $DIMENSION

echo "Hybrid (static) - Starting dynamic strategy"
./run_mandle_hybrid_static.sh 8 8 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 8 100 2 $DIMENSION $DIMENSION

./run_mandle_hybrid_static.sh 8 16 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_static.sh 16 16 100 2 $DIMENSION $DIMENSION

cp output.csv $OUTPUT/output_omp_static.csv

echo "Strategy,Workers,Threads,Dimension (width x height),Time" > output.csv

echo "Hybrid (dynamic) - Starting static strategy"
./run_mandle_hybrid_dynamic.sh 8 8 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 8 100 0 $DIMENSION $DIMENSION

./run_mandle_hybrid_dynamic.sh 8 16 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 16 100 0 $DIMENSION $DIMENSION

echo "Hybrid (dynamic) - Starting static strategy with round-robin"
./run_mandle_hybrid_dynamic.sh 8 8 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 8 100 1 $DIMENSION $DIMENSION

./run_mandle_hybrid_dynamic.sh 8 16 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 16 100 1 $DIMENSION $DIMENSION

echo "Hybrid (dynamic) - Starting dynamic strategy"
./run_mandle_hybrid_dynamic.sh 8 8 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 8 100 2 $DIMENSION $DIMENSION

./run_mandle_hybrid_dynamic.sh 8 16 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_dynamic.sh 16 16 100 2 $DIMENSION $DIMENSION

cp output.csv $OUTPUT/output_omp_dynamic.csv

echo "Strategy,Workers,Threads,Dimension (width x height),Time" > output.csv

echo "Hybrid (guided) - Starting static strategy"
./run_mandle_hybrid_guided.sh 8 8 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 8 100 0 $DIMENSION $DIMENSION

./run_mandle_hybrid_guided.sh 8 16 100 0 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 16 100 0 $DIMENSION $DIMENSION

echo "Hybrid (guided) - Starting static strategy with round-robin"
./run_mandle_hybrid_guided.sh 8 8 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 8 100 1 $DIMENSION $DIMENSION

./run_mandle_hybrid_guided.sh 8 16 100 1 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 16 100 1 $DIMENSION $DIMENSION

echo "Hybrid (guided) - Starting dynamic strategy"
./run_mandle_hybrid_guided.sh 8 8 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 8 100 2 $DIMENSION $DIMENSION

./run_mandle_hybrid_guided.sh 8 16 100 2 $DIMENSION $DIMENSION
./run_mandle_hybrid_guided.sh 16 16 100 2 $DIMENSION $DIMENSION

cp output.csv $OUTPUT/output_omp_guided.csv

date
echo "All test finished"