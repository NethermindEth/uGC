#!/bin/bash

export TOPDIR="$(cd "$(dirname "$(which "$0")")" ; pwd -P)"

if [ "${RUNTIME_BASE_DIR}" == "" ] ; then
    echo "RUNTIME_BASE_DIR is not set" >&2
    exit 1
fi

mkdir -p build
pushd build > /dev/null 2> /dev/null
    cmake -GNinja \
          -DUGC_RUNTIME_INCLUDE_DIR=${RUNTIME_BASE_DIR} \
          -DCMAKE_INSTALL_PREFIX=$(pwd)/usr \
          -DCMAKE_TOOLCHAIN_FILE=${TOPDIR}/toolchain/rv64ima.cmake \
          ..
    ninja
    ninja install
popd > /dev/null 2> /dev/null
