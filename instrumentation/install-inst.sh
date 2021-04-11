#!/bin/bash

set -e
#set -x
ROOT=$(readlink -f $(dirname $0))
BUILD_DIR="${ROOT}/build"
INSTALL_DIR="${ROOT}"

usage() {
    echo "usage: install.sh [ LLVM_ROOT | -s | -h ]"
}

detail() {
    usage
    echo ""
    echo "arguments:"
    echo "LLVM_ROOT  path to the installed directory of LLVM"
    echo "-s         use system LLVM to compile the instrumentation pass (the first llvm found in \$PATH)"
    echo "-h         show this help message and exit"
}

report_error() {
    echo "$1"
    usage
    exit 1
}

if [ $# -lt "1" ]; then
    usage
    exit 1
fi


case "$1" in
    -h) 
        detail
        exit 0
        ;;
    -s)
        CLANG_DIR=$(which clang)
        if [ ! -e ${CLANG_DIR} ]; then
            report_error "LLVM is not installed in the system"
        fi
        LLVM_ROOT=$(echo ${CLANG_DIR} | grep -Po "^.+(?=/bin/clang)")
        ;;
    -*)
        report_error "Unknown argument $1"
        ;;
    *)
        LLVM_ROOT=$(readlink -f $1)
        if [ ! -d ${LLVM_ROOT} ]; then
            report_error "$LLVM_ROOT is not a valid path"
        fi

        if [ ! -e "${LLVM_ROOT}/bin/clang" ]; then
            report_error "clang is not available in the LLVM (${LLVM_ROOT})"
        fi

esac

echo "LLVM path: ${LLVM_ROOT}"

CLANG="${LLVM_ROOT}/bin/clang"
CLANGPP="${LLVM_ROOT}/bin/clang++"

if [ -d ${BUILD_DIR} ]; then
    rm -rf ${BUILD_DIR}/*
else
    mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake -DSYS_LLVM_INSTALL_DIR="${LLVM_ROOT}"       \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"     \
      -DCMAKE_C_COMPILER="${CLANG}" \
      -DCMAKE_CXX_COMPILER="${CLANGPP}" ${ROOT}
make install

echo "Instrumentation pass has been installed into ${INSTALL_DIR}"
