#!/bin/bash
set -e

usage() {
  echo "run_bench.sh [--bench bench|noskip ] [-h]"
  echo "options:"
  echo "  --bench: benchmark set to be evaluated. The valid value is 'bench' or 'noskip'"
  echo "  -h:      print usage"
}

ROOT=$(readlink -f $(dirname $0)/..)
CURRENT_DIR=$(pwd)
BENCHMARKS_DEFAULT="health knapsack matmul poisson sort sparselu strassen"
BENCHMARKS_NOSKIP="matmul poisson sort sparselu strassen"

while [ $# -gt "0" ]; do
  case "$1" in
      -h)
      usage
      exit 1
      ;;
    --bench)
      shift
      if [ $# -eq "0" ]; then
        echo "Miss benchmark set name after '--bench'"
        exit 1
      fi
      BENCHMARK_SET=$1
      shift
      ;;
    *)
      echo "Unknown parameter $1"
      usage
      exit 1
  esac
done

if [ -z ${BENCHMARK_SET:+x} ]; then
 BENCHMARK_SET=bench
fi

if [ ${BENCHMARK_SET} == 'bench' ]; then
  BENCHMARKS=${BENCHMARKS_DEFAULT}
elif [ ${BENCHMARK_SET} == 'noskip' ]; then
  BENCHMARKS=${BENCHMARKS_NOSKIP}
else
  echo "Unknown benchmark set name '${BENCHMARK_SET}'"
  exit 1
fi

OUTPUT="${CURRENT_DIR}/run-${BENCHMARK_SET}-$(date +%y%m%d-%H%M%S).csv"

touch ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result
echo "Benchmark,Orig-Time(sec),Orig-Memory(kb),Rd-Time(sec),Rd-Memory(kb),Time Overhead,Memory Overhead" >> $OUTPUT

for bench in ${BENCHMARKS}; do 
  pushd ${ROOT}/${BENCHMARK_SET}/${bench} > /dev/null
   
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

