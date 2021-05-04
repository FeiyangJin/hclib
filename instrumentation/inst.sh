#!/bin/bash

set -e
#set -x

usage() {
    echo "usage: inst.sh [--llvm LLVM_ROOT] SOURCE_FILE [OPTIONS]"
}

report_error() {
    echo "$1"
    usage
    exit 1
}

ROOT=$(readlink -f $(dirname $0))
PASS_LIB="libinstrumentation.so"
DETECTOR="asap"
DETECTOR_LIB="lib${DETECTOR}.so"

if [ $# -lt "1" ]; then
    usage
    exit 1
fi

USE_SYSTEM_LLVM=1
while [ $# -gt "0" ]; do
    case "$1" in
        --llvm)
            shift
            if [ $# -lt "1" ]; then
                report_error "Miss path to the LLVM"
            fi
            LLVM_ROOT=$1
            USE_SYSTEM_LLVM=0
            shift
            ;;
        -*)
            report_error "Unknown argument $1"
            ;;
        *)
            if [ ! -f $1 ]; then
                report_error "$(readlink -f $1) is not a vaild file"
            fi
            if [ ${1: -2} != ".c" ] && [ ${1: -4} != ".cpp" ]; then
                report_error "$(readlink -f $1) is not a valid c/cpp source file"
            fi
            SOURCE_FILE=$1
            shift
            OPTIONS=$*
            shift $#
    esac
done 

if [ -z ${SOURCE_FILE:+x} ]; then
    report_error "SOURCE_FILE is not specified"
fi

if [ ${USE_SYSTEM_LLVM} == "1" ]; then
    CLANG=$(which clang)
    CLANGPP=$(which clang++)
    OPT=$(which opt)
else
    CLANG=${LLVM_ROOT}/bin/clang
    CLANGPP=${LLVM_ROOT}/bin/clang++
    OPT=${LLVM_ROOT}/bin/opt
fi

if [ ! -e ${CLANG} ]; then
    report_error "clang is not available"
fi

echo "================== Install Instrumentation Pass ==============================="
pushd ${ROOT} > /dev/null 2>&1
if [ ! -e "${PASS_LIB}" ]; then
    echo "Instrumentation pass is not found, try to install it using install-inst.sh"
    if [ ${USE_SYSTEM_LLVM} == "1" ]; then
        ${ROOT}/install-inst.sh -s
    else
        ${ROOT}/install-inst.sh ${LLVM_PATH}
    fi
    echo "Install instrumentation pass successfully"
else
    echo "Instrumentation pass exists, skip the installation"
fi

if [ ! -e "${DETECTOR_LIB}" ]; then
    ${CLANG} -c -o check.o check.c
    ${CLANG} -shared -fpic -o ${DETECTOR_LIB} check.o
fi

echo "==============================================================================="
echo ""
echo "================== Instrument Source File ====================================="
# The check in line 44 guarantees that SOURCE_FILE must end with .c or .cpp
popd > /dev/null 2>&1
if [ ${SOURCE_FILE: -2} == ".c" ]; then
    BC=${SOURCE_FILE/%.c/.bc}
    INST_BC=${SOURCE_FILE/%.c/-inst.bc}
    EXE=${SOURCE_FILE/%.c/.exe}
    echo "${CLANG} -c -g -emit-llvm ${OPTIONS} -o ${BC} ${SOURCE_FILE}"
    ${CLANG} -c -g -emit-llvm ${OPTIONS} -o ${BC} ${SOURCE_FILE}
    echo "${OPT} -load-pass-plugin ${ROOT}/${PASS_LIB} --passes=\"asap-inst\" -o ${INST_BC} ${BC}"
    ${OPT} -load-pass-plugin ${ROOT}/${PASS_LIB} --passes="asap-inst" -o ${INST_BC} ${BC}
    echo "${CLANG} ${INST_BC} -L${ROOT} -lasap ${OPTIONS} -o ${EXE}"
    ${CLANG} ${INST_BC} -L${ROOT} -lasap ${OPTIONS} -o ${EXE}
else
    BC=${SOURCE_FILE/%.cpp/.bc}
    INST_BC=${SOURCE_FILE/%.cpp/-inst.bc}
    EXE=${SOURCE_FILE/%.cpp/.exe}
    echo "${CLANGPP} -c -g -emit-llvm ${OPTIONS} -o ${BC} ${SOURCE_FILE}"
    ${CLANGPP} -c -g -emit-llvm ${OPTIONS} -o ${BC} ${SOURCE_FILE}
    echo "${OPT} -load-pass-plugin ${ROOT}/${PASS_LIB} --passes=\"asap-inst\" -o ${INST_BC} ${BC}"
    ${OPT} -load-pass-plugin ${ROOT}/${PASS_LIB} --passes="asap-inst" -o ${INST_BC} ${BC}
    echo "${CLANGPP} ${INST_BC} -L${ROOT} -lasap ${OPTIONS} -o ${EXE}"
    ${CLANGPP} ${INST_BC} -L${ROOT} -lasap ${OPTIONS} -o ${EXE}
fi

echo "==============================================================================="
echo ""
echo "Instrument successfully. The instrumented execuable is $(pwd)/${EXE}"
echo "To execute the executable, use following command:" 
echo "    LD_LIBRARY_PATH=\"${ROOT}:"'${LD_LIBRARY_PATH}" ' "./${EXE}"