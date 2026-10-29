# rv32im
#
# RV32IM ilp32 (soft-float, no C/A extensions) with clang, for the ZKsync
# Airbender guests of bflat. The compiles take the headers of an rv32im musl
# sysroot, RV32_SYSROOT, that has the libc++ headers in usr/include/c++/v1:
# the one that dotnet-riscv builds the riscv32 runtime against
# (build_musl_rv32im.sh, build_libcxx_headers_rv32.sh).
#
# No linker relaxation: "ld -r" keeps the R_RISCV_ALIGN padding of the
# objects, which can then leave code on a 2-byte boundary, and an IALIGN=32
# core traps there.

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_SYSTEM_PROCESSOR RISCV32)

if(NOT DEFINED RV32_SYSROOT)
    set(RV32_SYSROOT "$ENV{RV32_SYSROOT}")
endif()
if(NOT EXISTS "${RV32_SYSROOT}/usr/include/c++/v1")
    message(FATAL_ERROR "RV32_SYSROOT must be an rv32im musl sysroot with the libc++ headers")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(OBJECT_GEN_FLAGS "--target=riscv32-unknown-linux-musl --sysroot=${RV32_SYSROOT} -march=rv32im -mabi=ilp32 -mno-relax -mcmodel=medany -ffunction-sections -fdata-sections -fno-stack-protector ")

set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu99 " CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -nostdinc++ -isystem ${RV32_SYSROOT}/usr/include/c++/v1 " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} " CACHE INTERNAL "ASM Compiler options")

set(CMAKE_C_COMPILER clang CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER clang++ CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER clang CACHE INTERNAL "ASM Compiler")
set(CMAKE_AR llvm-ar CACHE INTERNAL "Archiver")
set(CMAKE_RANLIB llvm-ranlib CACHE INTERNAL "Ranlib")

set(CMAKE_FIND_ROOT_PATH ${RV32_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
