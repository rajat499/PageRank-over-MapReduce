#!/bin/bash

# Usage
# ./check.sh [cores] [input_file] [expected_output_file]

echo "File $2 with $1 cores."
mpirun -n $1 ./pagerank.o $2 _temp
python3 difference.py _temp $3
rm _temp
echo