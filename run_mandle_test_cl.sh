#!/bin/sh

OUTPUT=output_cl
rm -rf $OUTPUT/*
mkdir -p $OUTPUT

DIMENSION=10000

# Create the CSV header
echo "Time,Samples/seconds,Dimension (width x height)" > output_cl.csv

echo "Starting static strategy"
./run_mandle_cl.sh 100 $DIMENSION $DIMENSION

# Copy MPI csv to a different location
cp output_cl.csv $OUTPUT/output_cl.csv

date
echo "All test finished"