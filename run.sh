#!/bin/bash
set -e

cd build
for p in add div mem nbody; do
    echo "Running $p..."
    python ../time.py ./$p -m `nproc`
done

NUM_CORES=`grep '^cpu cores' /proc/cpuinfo | uniq |  awk '{print $4}'`
for p in add_mem div_add div_mem nbody_add nbody_mem; do
    echo "Running $p..."
    python ../time.py ./$p -m $NUM_CORES
done
