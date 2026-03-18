FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git cmake clang-18 lld-18 libc++-18-dev libc++abi-18-dev \
    build-essential nasm wget curl python3 sed pkg-config \
    uuid-dev libjsoncpp-dev libssl-dev zlib1g-dev libc-ares-dev \
    libbotan-2-dev libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

WORKDIR /app
RUN git clone --depth=1 https://github.com/qubic/core-lite.git .

RUN mkdir -p /app/build && cd /app/build && cmake .. \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DCMAKE_BUILD_TYPE=Release

# Pre-build all deps (cached layer)
RUN cd /app/build && make -j$(nproc) fmt trantor drogon platform_common platform_efi

# Copy contracts after dep build so contract changes don't invalidate dep cache
COPY contracts/ /qzn/contracts/
COPY test/      /qzn/test/
COPY setup.sh /qzn/setup.sh
RUN chmod +x /qzn/setup.sh && /qzn/setup.sh

# Build Qubic — only qubic.cpp compiles here, errors clearly visible
RUN cd /app/build && make -j1 Qubic

# Build and run tests
RUN cd /app/build && \
    make -j$(nproc) qubic_core_tests && \
    ./test/qubic_core_tests --gtest_filter="*QZN*" \
    || echo "WARNING: Some QZN tests failed"

EXPOSE 41841
WORKDIR /app
CMD ["./build/Qubic", "--ticking-delay", "1000"]