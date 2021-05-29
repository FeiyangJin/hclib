#!/bin/bash
set -e

ROOT=$(readlink -f $(dirname $0)/..)
CURRENT_DIR=$(pwd)
BENCHMARKS="fib health matmul nqueens poisson sort sparselu strassen"
OUTPUT="${CURRENT_DIR}/run-$(date +%y%m%d-%H%M%S).csv"

touch ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result
echo "Benchmark,Orig-Time(sec),Orig-Memory(kb),Rd-Time(sec),Rd-Memory(kb),Time Overhead,Memory Overhead" >> $OUTPUT

for bench in ${BENCHMARKS}; do 
    pushd $ROOT/$bench > /dev/null
     
    NAME="${bench}-origin.exe"
    echo "Execute ${NAME}"
    ORIGIN_RESULT=$(./run.sh --orig |& awk '{ if ($0 ~ /^Time:/) { time=$2 } else if ($0 ~ /^Memory:/) { memory=$2 } } END { print time "," memory }')
    ORIGIN_TIME=$(echo ${ORIGIN_RESULT} | cut -d ',' -f 1)
    ORIGIN_MEMORY=$(echo ${ORIGIN_RESULT} | cut -d ',' -f 2)
    NAME="${bench}-rd.exe"
    echo "Execute ${NAME}"
    RD_RESULT=$(./run.sh --rd |& awk '{ if ($0 ~ /^Time:/) { time=$2 } else if ($0 ~ /^Memory:/) { memory=$2 } } END { print time "," memory }')
    RD_TIME=$(echo ${RD_RESULT} | cut -d ',' -f 1)
    RD_MEMORY=$(echo ${RD_RESULT} | cut -d ',' -f 2)
    TIME_OVERHEAD=$(echo "scale=2; ${RD_TIME} / ${ORIGIN_TIME}" | bc)
    MEMORY_OVERHEAD=$(echo "scale=2; ${RD_MEMORY} / ${ORIGIN_MEMORY}" | bc)
    echo "${bench},${ORIGIN_TIME},${ORIGIN_MEMORY},${RD_TIME},${RD_MEMORY},${TIME_OVERHEAD},${MEMORY_OVERHEAD}" >> $OUTPUT
    popd > /dev/null
done

