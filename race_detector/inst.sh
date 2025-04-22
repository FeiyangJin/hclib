#!/bin/bash

set -e
#set -x

usage() {
  echo "usage: inst.sh [--llvm LLVM_ROOT] [-o OUTPUT] SOURCE_FILE -- [OPTIONS_FOR_LLVM]"
}

report_error() {
  echo "$1"
  usage
  exit 1
}

ROOT=$(readlink -f $(dirname $0))
HCLIB_LIB_PATH=$(readlink -f ${ROOT}/../lib)
PASS_LIB="libinstrumentation.so"
DETECTOR="drdp"
DETECTOR_LIB="lib${DETECTOR}.so"

LLVM_LIB_PATH="/storage/pace-apps/spack/packages/linux-rhel9-x86_64_v3/gcc-11.3.1/llvm-16.0.2-ybmhd4ai6gg5n3pt4tv5qoly3nrc5tun/lib"

# Adjusted order to ensure symbols are resolved
LINK_OPTION1="-lLLVMSupport -lLLVMSymbolize -lLLVMDebugInfoDWARF -lLLVMDebugInfoPDB -lLLVMDebugInfoMSF -lLLVMObject -lLLVMBitReader -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMTextAPI ${LLVM_LIB_PATH}/libLLVMBinaryFormat.a ${LLVM_LIB_PATH}/libLLVMTarget.a ${LLVM_LIB_PATH}/libLLVMTargetParser.a ${LLVM_LIB_PATH}/libLLVMAnalysis.a -lLLVMAsmParser -lLLVMDemangle -lrt -ldl -lpthread -lm /usr/lib64/libz.so /usr/lib64/libtinfo.so"

# Find llvm-config corresponding to the clang we're using
if [ ${USE_SYSTEM_LLVM} == "1" ]; then
  LLVM_CONFIG=$(which llvm-config)
  LLVM_LIB_PATH=$(${LLVM_CONFIG} --libdir)
else
  LLVM_CONFIG=${LLVM_ROOT}/bin/llvm-config
  LLVM_LIB_PATH=${LLVM_ROOT}/lib
fi

# Use llvm-config to get all necessary libraries
if [ -x "$LLVM_CONFIG" ]; then
  LINK_OPTION2="$(${LLVM_CONFIG} --libs all) -lrt -ldl -lpthread -lm /usr/lib64/libz.so /usr/lib64/libtinfo.so"
else
  # Fallback if llvm-config isn't available
  LINK_OPTION2="-lLLVMSupport -lLLVMTargetParser -lLLVMAsmParser -lLLVMSymbolize -lLLVMDebugInfoDWARF -lLLVMDebugInfoPDB -lLLVMDebugInfoMSF -lLLVMObject -lLLVMIRReader -lLLVMBitReader -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMTextAPI -lLLVMBinaryFormat -lLLVMTarget -lLLVMAnalysis -lLLVMDemangle -lrt -ldl -lpthread -lm /usr/lib64/libz.so /usr/lib64/libtinfo.so"
fi

# Set LD_LIBRARY_PATH to ensure runtime libraries can be found
export LD_LIBRARY_PATH="${LLVM_LIB_PATH}:${HCLIB_LIB_PATH}:$LD_LIBRARY_PATH"

#DEFAULT_OPTIONS="-g"
DEFAULT_OPTIONS=""

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
    -o)
      shift
      if [ $# -lt "1" ]; then
          report_error "Miss Output file for option -o"
      fi
      OUTPUT=$1
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
      if [ $1 == '--' ]; then
        shift
      fi
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

# Replace the CLANG_VERSION section with:
CLANG_VERSION=$(${CLANG} --version | grep -oP 'clang version \K[0-9]+' || echo "0")
echo "Clang major version: ${CLANG_VERSION}"
if [ ${CLANG_VERSION} -lt 14 ]; then
  report_error "clang's minimum required version is 14"
elif [ ${CLANG_VERSION} -eq 14 ]; then
  LINK_OPTION=${LINK_OPTION1}
else
  LINK_OPTION=${LINK_OPTION2}
fi

LLVM_LIB="${CLANG/bin\/clang/lib}"

# # Check and install DRDP
# echo "================== Install Instrumentation Pass ==============================="
# pushd ${ROOT} > /dev/null 2>&1
# if [ ! -e "${PASS_LIB}" ]; then
#     echo "Instrumentation pass is not found, try to install it using install-inst.sh"
#     if [ ${USE_SYSTEM_LLVM} == "1" ]; then
#         ${ROOT}/install-inst.sh -s
#     else
#         ${ROOT}/install-inst.sh ${LLVM_PATH}
#     fi
#     echo "Install instrumentation pass successfully"
# else
#     echo "Instrumentation pass exists, skip the installation"
# fi

# if [ ! -e "${DETECTOR_LIB}" ]; then
#     ${CLANG} -c -o check.o check.c
#     ${CLANG} -shared -fpic -o ${DETECTOR_LIB} check.o
# fi

# echo "==============================================================================="
# echo ""
# popd > /dev/null 2>&1


# The check in line 44 guarantees that SOURCE_FILE must end with .c or .cpp
echo "================== Instrument Source File ====================================="
if [ ${SOURCE_FILE: -2} == ".c" ]; then
  BC=${SOURCE_FILE/%.c/.bc}
  INST_BC=${SOURCE_FILE/%.c/-inst.bc}
  EXE=${OUTPUT:=${SOURCE_FILE/%.c/.exe}}
  echo "${CLANG} -c -emit-llvm ${DEFAULT_OPTIONS} ${OPTIONS} -o ${BC} ${SOURCE_FILE}"
  ${CLANG} -c -emit-llvm ${DEFAULT_OPTIONS} ${OPTIONS} -o ${BC} ${SOURCE_FILE}
  echo "${OPT} -load-pass-plugin ${HCLIB_LIB_PATH}/${PASS_LIB} --passes=\"${DETECTOR}-inst\" -o ${INST_BC} ${BC}"
  ${OPT} -load-pass-plugin ${HCLIB_LIB_PATH}/${PASS_LIB} --passes="${DETECTOR}-inst" -o ${INST_BC} ${BC}
  # echo "${CLANG} -L${HCLIB_LIB_PATH} -L${LLVM_LIB} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}"
  # ${CLANG} -L${HCLIB_LIB_PATH} -L${LLVM_LIB} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}
  COMPILE_CMD="${CLANG} -L${HCLIB_LIB_PATH} -L${LLVM_LIB_PATH} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}"
  echo "$COMPILE_CMD"
  $COMPILE_CMD
else
  BC=${SOURCE_FILE/%.cpp/.bc}
  INST_BC=${SOURCE_FILE/%.cpp/-inst.bc}
  EXE=${OUTPUT:=${SOURCE_FILE/%.cpp/.exe}}
  echo "${CLANGPP} -c -emit-llvm ${DEFAULT_OPTIONS} ${OPTIONS} -o ${BC} ${SOURCE_FILE}"
  ${CLANGPP} -c -emit-llvm ${DEFAULT_OPTIONS} ${OPTIONS} -o ${BC} ${SOURCE_FILE}
  echo "${OPT} -load-pass-plugin ${HCLIB_LIB_PATH}/${PASS_LIB} --passes=\"${DETECTOR}-inst\" -o ${INST_BC} ${BC}"
  ${OPT} -load-pass-plugin ${HCLIB_LIB_PATH}/${PASS_LIB} --passes="${DETECTOR}-inst" -o ${INST_BC} ${BC}
  # echo "${CLANGPP} -L${HCLIB_LIB_PATH} -L${LLVM_LIB} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}"
  # ${CLANGPP} -L${HCLIB_LIB_PATH} -L${LLVM_LIB} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}
  COMPILE_CMD="${CLANGPP} -L${HCLIB_LIB_PATH} -L${LLVM_LIB_PATH} ${INST_BC} -l${DETECTOR} ${LINK_OPTION} ${DEFAULT_OPTIONS} ${OPTIONS} -o ${EXE}"
  echo "$COMPILE_CMD"
  $COMPILE_CMD
fi

echo "==============================================================================="
echo ""
echo "Instrument successfully. The instrumented execuable is $(pwd)/${EXE}"
echo "To execute the executable, use following command:" 
#echo "    LD_LIBRARY_PATH=\"${ROOT}:"'${LD_LIBRARY_PATH}" ' "./${EXE}"
echo "./${EXE}"
