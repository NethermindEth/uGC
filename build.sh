#!/bin/bash
#
# ./build.sh [docker] [riscv64|riscv32]
#
# riscv64 (the default) builds build/usr/lib/libugc-zero.a and its objects
# with the riscv64 GCC. riscv32 builds build-riscv32/usr/lib/libugc-zero.a,
# its objects and ugc.o, all of them as one relocatable object, for the
# RV32IM guests of bflat, with clang against RV32_SYSROOT
# (toolchain/rv32im.cmake). "docker" builds in the image of the target
# (Dockerfile, Dockerfile.riscv32).

export TOP_DIR="$(cd "$(dirname "$0")" ; pwd -P)"

is_docker=
arch=riscv64
for arg in "$@" ; do
    case "$arg" in
        docker) is_docker=docker ;;
        riscv64|riscv32) arch="$arg" ;;
        *) echo "usage: $0 [docker] [riscv64|riscv32]" >&2 ; exit 1 ;;
    esac
done

pushd "${TOP_DIR}"
    if [ "$is_docker" == "docker" ] ; then
        if [ "$arch" == "riscv32" ] ; then
            dockerfile=Dockerfile.riscv32
            platform="--platform linux/amd64"
        else
            dockerfile=Dockerfile
            platform=
        fi
        docker build ${platform} -f ${dockerfile} .
        sha=$(docker build ${platform} -q -f ${dockerfile} .)
        echo Current directory: $(pwd)
        echo Command: docker run ${platform} -v $(pwd):/workspace -w /workspace $sha ./build.sh ${arch}
        echo Current directory content:
        docker run ${platform} -v $(pwd):/workspace -w /workspace $sha ls -all
        echo "-- end of content"
        docker run ${platform} -v $(pwd):/workspace -w /workspace $sha ./build.sh ${arch}
        exit $?
    fi

    if [ "${RUNTIME_BASE_DIR}" == "" ] ; then
        echo "RUNTIME_BASE_DIR is not set" >&2
        exit 1
    fi

    if [ "$arch" == "riscv32" ] ; then
        if [ "${RV32_SYSROOT}" == "" ] ; then
            echo "RV32_SYSROOT is not set" >&2
            exit 1
        fi

        mkdir -p build-riscv32
        pushd build-riscv32 > /dev/null 2> /dev/null
            cmake -GNinja \
                -DUGC_RUNTIME_INCLUDE_DIR=${RUNTIME_BASE_DIR} \
                -DRV32_SYSROOT=${RV32_SYSROOT} \
                -DCMAKE_INSTALL_PREFIX=$(pwd)/usr \
                -DCMAKE_TOOLCHAIN_FILE=${TOP_DIR}/toolchain/rv32im.cmake \
                .. || exit 1
            ninja || exit 1
            ninja install || exit 1
            # The guests link one object: the objects of the library, put
            # together.
            ld.lld -r -o usr/lib/ugc.o $(find usr/lib/objects/ugc-zero -name '*.obj' | sort) || exit 1
            # The guests decode RV32IM only (no compressed, atomic or fence
            # instructions): see tools/isa_scan.py of dotnet-riscv.
            if [ "${ISA_SCAN}" != "" ] ; then
                OBJDUMP=llvm-objdump ISA_SCAN_FENCE=1 python3 "${ISA_SCAN}" usr/lib/ugc.o || exit 1
            fi
        popd > /dev/null 2> /dev/null
        exit 0
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
