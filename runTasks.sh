done#! /usr/bin/bash

# Exit immediately if a command fails 
set -e

# Check if a.out exists and is executable
if [ ! -x "./a.out" ]; then
    echo "Error: a.out not found or not executable"
    exit 1
fi

for i in ./task1/*.bin; do
    if [ -f "$i" ]; then
        echo "Running a.out with $i"
        ./a.out "$i"
    else
        echo "No .bin files found"
    fi
done
