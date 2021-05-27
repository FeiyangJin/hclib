#!/bin/bash
set -e
#set -x

usage() {
    echo "setup_benchmark [space_separated_benchmark_names]"
}

ROOT=$(readlink -f $(dirname $0))
MAKEFILE_TEMPLATE="${ROOT}/Makefile.template"
MAKEFILE_OUTPUT="Makefile"
RUN_SH_TEMPLATE="${ROOT}/run.template"
RUN_SH_OUTPUT="run.sh"

if [ $# -lt 1 ]; then
    usage
    exit 1
fi

for BENCHMARK_NAME in $*; do
    echo "Set up ${BENCHMARK_NAME}"
    DEFAULT_INPUT=""
    if [ ! -d ${BENCHMARK_NAME} ]; then
        mkdir ${BENCHMARK_NAME}
    elif [ -e ${BENCHMARK_NAME}/${RUN_SH_OUTPUT} ]; then
        DEFAULT_INPUT=$(awk 'BEGIN{FS="="} /DEFAULT_INPUT=/ {print substr($2, 2, length($2)-2)}' ${BENCHMARK_NAME}/${RUN_SH_OUTPUT})
    fi
    echo "Default input: ${DEFAULT_INPUT}"
    pushd ${BENCHMARK_NAME} > /dev/null
    sed "s/#name/${BENCHMARK_NAME}/g" ${MAKEFILE_TEMPLATE} > ${MAKEFILE_OUTPUT}
    sed "s/#name/${BENCHMARK_NAME}/g; s/#input/${DEFAULT_INPUT}/g" ${RUN_SH_TEMPLATE} > ${RUN_SH_OUTPUT}
    popd > /dev/null
done

