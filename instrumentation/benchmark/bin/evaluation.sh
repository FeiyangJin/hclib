#!/bin/bash
set -e

ROOT=$(readlink -f $(dirname $0)/..)
CURRENT_DIR=$(pwd)
BENCHMARKS="fib health matmul nqueens poisson sort sparselu"
EXPERIMENTS="orig rd"
TIMES='5'
OUTPUT="${CURRENT_DIR}/eval-$(date +%y%m%d-%H%M%S).csv"
touch ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result

HEAD="Benchmark"
for e in ${EXPERIMENTS}; do
    for i in $(seq 1 1 ${TIMES}); do
        HEAD="${HEAD},${e^}-Time-${i} (sec),${e^}-Memory-${i} (kb)"
    done
done
echo "${HEAD}" >> ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result

for bench in ${BENCHMARKS}; do 
    OUTPUT_LINE="${bench}"
    pushd $ROOT/$bench > /dev/null
    for e in ${EXPERIMENTS}; do
        echo "Run ${bench} ${e}-version ${TIMES} times"
        for i in $(seq 1 1 ${TIMES}); do
            RESULT=$(./run.sh --${e} |& awk '{ if ($0 ~ /^Time:/) { time=$2 } else if ($0 ~ /^Memory:/) { memory=$2 } } END { print time "," memory }')
            OUTPUT_LINE="${OUTPUT_LINE},${RESULT}"
            echo "Execution ${i} is done"
        done
    done
    echo "${OUTPUT_LINE}" >> ${OUTPUT}
    popd > /dev/null
done

python3 ${ROOT}/bin/analysis.py ${OUTPUT}