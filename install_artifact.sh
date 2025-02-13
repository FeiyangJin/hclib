#!/bin/bash
set -e

ROOT=$(readlink -f $(dirname $0))

pushd ${ROOT} > /dev/null

# install original hclib
INSTALL_PREFIX="${ROOT}/hclib-install-orig" ${ROOT}/install.sh -DHCLIB_ENABLE_PRODUCTION=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# install hclib with DRDP
INSTALL_PREFIX="${ROOT}/hclib-install" ${ROOT}/install.sh -DHCLIB_ENABLE_DRDP=ON -DHCLIB_DRDP_DEBUG_INFO=ON -DHCLIB_ENABLE_PRODUCTION=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

popd > /dev/null
