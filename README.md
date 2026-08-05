# uGC

[![CI](https://github.com/NethermindEth/uGC/actions/workflows/ci.yml/badge.svg)](https://github.com/NethermindEth/uGC/actions/workflows/ci.yml)
[![Release](https://github.com/NethermindEth/uGC/actions/workflows/release.yml/badge.svg)](https://github.com/NethermindEth/uGC/actions/workflows/release.yml)
[![License](https://img.shields.io/github/license/NethermindEth/uGC)](LICENSE)
[![Latest Release](https://img.shields.io/github/v/release/NethermindEth/uGC)](https://github.com/NethermindEth/uGC/releases/latest)

Lightweight Garbage Collector for .NET on RISC-V 64-bit.

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

## License

This project is licensed under the terms of MIT license.

## Related Projects

- [bflat for RISC-V](https://github.com/NethermindEth/bflat-riscv64) - Native AOT compiler for .NET on RISC-V
