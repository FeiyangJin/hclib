#!/bin/bash
set -e

usage() {
  echo "evaluation.sh [--bench bench|noskip ] [-h]"
  echo "options:"
  echo "  --bench: benchmark set to be evaluated. The valid value is 'bench' or 'noskip'"
  echo "  -h:      print usage"
}

ROOT=$(readlink -f $(dirname $0)/..)
CURRENT_DIR=$(pwd)
BENCHMARKS_DEFAULT="health knapsack matmul poisson sort sparselu strassen"
BENCHMARKS_NOSKIP="matmul poisson sort sparselu strassen"
EXPERIMENTS="orig rd"

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

TIMES=${TIMES:=3}
OUTPUT="${CURRENT_DIR}/eval-$(date +%y%m%d-%H%M%S).csv"
touch ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result

HEAD="Benchmark"
for e in ${EXPERIMENTS}; do
  for i in $(seq 1 1 ${TIMES}); do
    HEAD="${HEAD},${e^}-Time-${i} (sec),${e^}-Memory-${i} (kB)"
  done
done
echo "${HEAD}" >> ${OUTPUT}
ln -fs ${OUTPUT} ${CURRENT_DIR}/latest-result

for bench in ${BENCHMARKS}; do 
  OUTPUT_LINE="${bench}"
  pushd ${ROOT}/${BENCHMARK_SET}/${bench} > /dev/null
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
