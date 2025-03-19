#!/bin/bash
set -e

usage() {
  echo "run.sh"
  echo "options:"
  exit 1
}

#if [ $# -lt 1 ]; then
#  usage
#  exit 1
#fi

BENCHMARK_NAME="test"
CURR=$(readlink -f $(dirname $0))
ROOT=${CURR/%\/race_detector_benchmark\/*/}
ENABLE_RACE_DETECTION=1
echo "Run race detection"

while [ $# -gt "0" ]; do
  case "$1" in
    -*)
      echo "Unknown option $1"
      usage
      exit 1
      ;;
    *)
      echo "Unknown parameter $1"
      usage
      exit 1
  esac
done

#if [ -z ${HCLIB_ROOT:+x} ]; then
RUNTIME="${ROOT}/hclib-install"

if [ ! -d ${RUNTIME} ]; then
  INSTALL_SCRIPT=${ROOT}/install_artifact.sh
  echo "hclib runtime does not exist!"
  echo "Please install the runtime through ${INSTALL_SCRIPT} before invoke this benchmark"
  exit 1
fi
source ${RUNTIME}/bin/hclib_setup_env.sh
#fi

failed_tests=()

for i in {1..29}; do
  RD_EXE="bin/${BENCHMARK_NAME}${i}-rd.exe"
  EXPECTED_OUTPUT="test_expected/test${i}.expected"
  ACTUAL_OUTPUT="test_output/test${i}.output"
  
  if [ ! -e ${RD_EXE} ]; then
      make ${RD_EXE} > /dev/null 2>&1
  fi
  
  HCLIB_WORKERS=1 ./${RD_EXE} > ${ACTUAL_OUTPUT}
  
  if ! diff ${EXPECTED_OUTPUT} ${ACTUAL_OUTPUT} > /dev/null; then
    failed_tests+=($i)
  fi
done

if [ ${#failed_tests[@]} -eq 0 ]; then
  echo "All tests have passed."
else
  echo "Tests ${failed_tests[@]} have failed."
fi