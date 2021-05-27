#!/bin/bash
set -e

usage() {
    echo "run.sh [--rd | --orig] [-- input_parameters]"
    echo "options:"
    echo "  --rd:     conduct data race detection"
    echo "  --orig:   conduct native execution"
    exit 1
}

if [ $# -lt 1 ]; then
    usage
    exit 1
fi

BENCHMARK_NAME="sort"
DEFAULT_INPUT="1000000"
ORIGIN_EXE="${BENCHMARK_NAME}-origin.exe"
RD_EXE="${BENCHMARK_NAME}-rd.exe"

while [ $# -gt "0" ]; do
    case "$1" in
        --rd)
            ENABLE_RACE_DETECTION=1
            shift
            ;;
        --orig)
            ENABLE_RACE_DETECTION=0
            shift
            ;;
        --)
            shift
            INPUT=$*
            shift $#
            ;;
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

if [ -z ${ENABLE_RACE_DETECTION:+x} ]; then
    echo "Miss the execution mode. Please specify --rd or --orig"
    usage
    exit 1
fi

if [ -z ${INPUT+x} ]; then
    echo "Use default input: ${DEFAULT_INPUT}"
    INPUT=${DEFAULT_INPUT}
else
    echo "Use specified input: ${INPUT}"
fi

#if [ -z ${HCLIB_ROOT:+x} ]; then
if [ ${ENABLE_RACE_DETECTION} -eq '0' ]; then
    source /home/fjin/hclib/hclib-install/bin/hclib_setup_env.sh
else
    source ../../../hclib-install/bin/hclib_setup_env.sh
fi
#fi

if [ ${ENABLE_RACE_DETECTION} -eq '0' ]; then
    if [ ! -e ${ORIGIN_EXE} ]; then
        make ${ORIGIN_EXE}
    fi
    echo "Run original application"
    echo "LD_LIBRARY_PATH=\"${LD_LIBRARY_PATH}\" HCLIB_WORKERS=1 ./${ORIGIN_EXE} ${INPUT}"
    HCLIB_WORKERS=1 /usr/bin/time -f "\nTime: %e sec\nMemory: %M kb" ./${ORIGIN_EXE} ${INPUT}
else
    if [ ! -e ${RD_EXE} ]; then
        make ${RD_EXE}
    fi
    echo "Run race detection"
    echo "LD_LIBRARY_PATH=\"../../../instrumentation:${LD_LIBRARY_PATH}\" HCLIB_WORKERS=1 ./${RD_EXE} ${INPUT}"
    LD_LIBRARY_PATH="../../../instrumentation:${LD_LIBRARY_PATH}" HCLIB_WORKERS=1 /usr/bin/time -f "\nTime: %e sec\nMemory: %M kb" ./${RD_EXE} ${INPUT}
fi
