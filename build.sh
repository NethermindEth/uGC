#!/bin/bash

export TOP_DIR="$(cd "$(dirname "$(which "$0")")" ; pwd -P)"

is_docker="$1"

pushd "${TOP_DIR}"
    if [ "$is_docker" == "docker" ] ; then
        docker build .
        sha=$(docker build -q .)
        echo Current directory: $(pwd)
        echo Command: docker run -v $(pwd):/workspace -w /workspace $sha ./build.sh
        echo Current directory content:
        docker run -v $(pwd):/workspace -w /workspace $sha ls -all
        echo "-- end of content"
        docker run -v $(pwd):/workspace -w /workspace $sha ./build.sh
        exit $?
    fi

    if [ "${RUNTIME_BASE_DIR}" == "" ] ; then
        echo "RUNTIME_BASE_DIR is not set" >&2
        exit 1
    fi

    which riscv64-linux-gnu-gcc
    export TOOLCHAIN_ROOT=/usr

    mkdir -p build
    pushd build > /dev/null 2> /dev/null
        cmake -GNinja \
            -DUGC_RUNTIME_INCLUDE_DIR=${RUNTIME_BASE_DIR} \
            -DCMAKE_INSTALL_PREFIX=$(pwd)/usr \
            -DCMAKE_TOOLCHAIN_FILE=${TOP_DIR}/toolchain/rv64ima.cmake \
            ..
        ninja
        ninja install
        file="usr/lib/libugc-zero.a"
        printf '\x00' | dd of="$file" bs=1 seek=$((0x30)) count=1 conv=notrunc
        pushd usr/lib/objects/ugc-zero
            for file in $(find . -name '*.obj') ; do
                printf '\x00' | dd of="$file" bs=1 seek=$((0x30)) count=1 conv=notrunc
            done
        popd
    popd > /dev/null 2> /dev/null
popd
