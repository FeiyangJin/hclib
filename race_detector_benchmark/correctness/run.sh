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

for i in {1..22}; do
  RD_EXE="${BENCHMARK_NAME}${i}-rd.exe"
  if [ ! -e ${RD_EXE} ]; then
      make ${RD_EXE} > /dev/null 2>&1
  fi
  HCLIB_WORKERS=1 /usr/bin/time -f "\nTime: %e sec\nMemory: %M kb" ./${RD_EXE}
done
