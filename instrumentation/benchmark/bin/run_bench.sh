#!/bin/bash
set -e

ROOT=$(readlink -f $(dirname $0)/..)
CURRENT_DIR=$(pwd)
BENCHMARKS="fib matmul nqueens sort"
OUTPUT="${CURRENT_DIR}/eval-$(date +%y%m%d-%H%M%S).csv"

echo "Benchmark,Time(sec),Memory(kb)" > $OUTPUT

for bench in ${BENCHMARKS}; do 
    pushd $ROOT/$bench > /dev/null
    
    NAME="${bench}-native"
    echo "Execute ${NAME}"
    RESULT=$(./run.sh --orig |& awk '{ if ($0 ~ /^Time:/) { time=$2 } else if ($0 ~ /^Memory:/) { memory=$2 } } END { print time "," memory }')
    echo "${NAME},${RESULT}" >> $OUTPUT
    
    NAME="${bench}-rd"
    echo "Execute ${NAME}"
    RESULT=$(./run.sh --rd |& awk '{ if ($0 ~ /^Time:/) { time=$2 } else if ($0 ~ /^Memory:/) { memory=$2 } } END { print time "," memory }')
    echo "${NAME},${RESULT}" >> $OUTPUT
    popd > /dev/null
done

ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result

