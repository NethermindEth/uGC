# uGC

[![CI](https://github.com/NethermindEth/uGC/actions/workflows/ci.yml/badge.svg)](https://github.com/NethermindEth/uGC/actions/workflows/ci.yml)
[![Release](https://github.com/NethermindEth/uGC/actions/workflows/release.yml/badge.svg)](https://github.com/NethermindEth/uGC/actions/workflows/release.yml)
[![License](https://img.shields.io/github/license/NethermindEth/uGC)](LICENSE)
[![Latest Release](https://img.shields.io/github/v/release/NethermindEth/uGC)](https://github.com/NethermindEth/uGC/releases/latest)

Lightweight Garbage Collector for .NET on RISC-V (64-bit, and RV32IM for the Airbender guests).

This is a custom GC implementation designed to be used with [Nethermind's **bflat for RISC-V 64 bit**](https://github.com/NethermindEth/bflat-riscv64).

## Features

- Optimized for RISC-V 64-bit architecture
- Zero-dependency static library
- Minimal overhead and memory footprint
- No actual free() implementation
- Core logic (allocation, handle store) implemented in plain C with ACSL
  contracts, formally verified with Frama-C (WP + Eva)

## Building

### Using Docker (Recommended)

The easiest way to build uGC is using the provided Docker environment:

```bash
./build.sh docker
```

This will:
1. Build a Docker image with all required dependencies
2. Compile the project using the RISC-V toolchain
3. Generate `build/usr/lib/libugc-zero.a` static library

### Manual Build

If you want to build manually, ensure you have:
- CMake 3.20 or higher
- Ninja build system
- RISC-V 64-bit GCC toolchain
- `RUNTIME_BASE_DIR` environment variable set

Then run:

```bash
./build.sh
```

### riscv32 (RV32IM)

The Airbender guests of bflat are RV32IM ilp32: no compressed, atomic,
floating-point or fence instructions. Their build uses clang
(`toolchain/rv32im.cmake`) against an rv32im musl sysroot with the libc++
headers, the one that dotnet-riscv builds the riscv32 runtime against:

```bash
./build.sh docker riscv32
```

`Dockerfile.riscv32` makes the sysroot with the dotnet-riscv scripts
(`build_musl_rv32im.sh`, `build_libcxx_headers_rv32.sh`) at
`DOTNET_RISCV_REF`, in the cross-build image of the .NET runtime that
dotnet-riscv uses. The build writes the objects to
`build-riscv32/usr/lib/objects/ugc-zero` and all of them, as one relocatable
object, to `build-riscv32/usr/lib/ugc.o`, which the guests link. It fails if
`ugc.o` carries an instruction that RV32IM does not have
(`tools/isa_scan.py` of dotnet-riscv). Outside Docker, set `RUNTIME_BASE_DIR`
and `RV32_SYSROOT`, and `ISA_SCAN` for the check:

```bash
RUNTIME_BASE_DIR=<runtime> RV32_SYSROOT=<sysroot> ./build.sh riscv32
```

## Architecture

All non-trivial logic lives in `ugc/core/` as dependency-free C11 code with
ACSL specifications (`ugc_core.h`). The C++ classes implementing the CLR GC
interface (`uGCHeap`, `uGCHandleStore`, `uGCHandleManager`) are thin wrappers
over that core.

## Testing

Unit tests cover the core allocator and handle store and run on the host
(no .NET runtime headers required):

```bash
cmake -S . -B build-tests -DUGC_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

## Formal Verification

The ACSL contracts of the core are verified with [Frama-C](https://frama-c.com):

- **WP** proves every contract (with runtime-error guards) deductively;
- **Eva** checks the absence of undefined behavior on a driver that
  exercises the core with unconstrained inputs.

Run the whole gate locally (requires Docker):

```bash
docker run --rm -v "$PWD":/work -w /work framac/frama-c:30.0 ./formal/verify.sh
```

CI fails if a single proof obligation is not discharged or Eva reports any
alarm.

## Signed releases

The release has two more assets: `bflat-signature.sha256`, the SHA-256 of
each other asset, and `bflat-signature.sha256.sig`, its signature. The
build job writes the list (`tools/sign_package.py list-files`). The sign
job, the reusable workflow `sign.yml` of bflat-riscv64, signs it with the
secret `BFLAT_SIGNING_KEY`; it is the only job that has the key, and it runs
no build. The release job checks the assets against the list and the
signature (`tools/sign_package.py check-files`, with the variable
`BFLAT_SIGNING_PUBLIC_KEY`) before it makes the release. bflat-riscv64
checks `ugc-riscv64.tar.gz` against them before it unpacks it
(`docs/signing.md` in bflat-riscv64). `tools/sign_package.py` is a copy of
the script in bflat-riscv64; keep it the same.

## License

This project is licensed under the terms of MIT license.

## Related Projects

- [bflat for RISC-V](https://github.com/NethermindEth/bflat-riscv64) - Native AOT compiler for .NET on RISC-V
