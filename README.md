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

## License

This project is licensed under the terms of MIT license.

## Related Projects

- [bflat for RISC-V](https://github.com/NethermindEth/bflat-riscv64) - Native AOT compiler for .NET on RISC-V
