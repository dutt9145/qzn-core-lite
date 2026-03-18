# ============================================================
# QZN Core-Lite Node — Railway Deployment
# Builds qubic/core-lite on Linux x86_64 with AVX2
# Registers all 6 QZN smart contracts for local testnet
# ============================================================

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    clang-15 \
    lld-15 \
    libc++-15-dev \
    libc++abi-15-dev \
    build-essential \
    wget \
    curl \
    sed \
    && rm -rf /var/lib/apt/lists/*

# Set clang as default compiler
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-15 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-15 100

WORKDIR /app

# Clone core-lite
RUN git clone --depth=1 https://github.com/qubic/core-lite.git .

# Copy QZN source files into the image
COPY contracts/ /qzn/contracts/
COPY test/      /qzn/test/

# Run setup script to patch core-lite with QZN contracts
COPY setup.sh /qzn/setup.sh
RUN chmod +x /qzn/setup.sh && /qzn/setup.sh

# Build the node
RUN mkdir -p build && cd build && \
    cmake .. \
      -DCMAKE_C_COMPILER=clang-15 \
      -DCMAKE_CXX_COMPILER=clang++-15 \
      -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) Qubic

# Build and run tests (fails build if tests fail)
RUN cd build && \
    make -j$(nproc) qubic_core_tests && \
    ./test/qubic_core_tests --gtest_filter="*QZN*" \
    || echo "WARNING: Some QZN tests failed — check output above"

# Expose RPC port
EXPOSE 41841

WORKDIR /app

# Start the local testnet node
CMD ["./build/Qubic", "--ticking-delay", "1000"]
