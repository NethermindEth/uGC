FROM ubuntu:25.04

RUN apt-get update -y && apt-get install -y build-essential gcc-riscv64-linux-gnu g++-riscv64-linux-gnu git ninja-build cmake

RUN git clone https://github.com/dotnet/runtime /runtime && cd /runtime && git checkout v10.0.0

ENV RUNTIME_BASE_DIR=/runtime
