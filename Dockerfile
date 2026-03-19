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
RUN sed -i 's/-mavx512f//g' /app/src/kangaroo_twelve.h || true
RUN sed -i 's/__AVX512F__/__AVX2__/g' /app/src/kangaroo_twelve.h || true
RUN sed -i 's/avx512/avx2/g' /app/src/kangaroo_twelve.h || true
RUN mkdir -p /app/build && cd /app/build && cmake .. \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-mavx2 -mno-avx512f" \
      -DCMAKE_C_FLAGS="-mavx2 -mno-avx512f"
RUN cd /app/build && make -j$(nproc) fmt trantor drogon platform_common platform_efi
COPY contracts/ /qzn/contracts/
COPY test/      /qzn/test/
COPY setup.sh /qzn/setup.sh
RUN chmod +x /qzn/setup.sh && /qzn/setup.sh
RUN cd /app/build && make -j$(nproc) Qubic
RUN cd /app/build && \
    make -j$(nproc) qubic_core_tests && \
    ./test/qubic_core_tests --gtest_filter="*QZN*" \
    || echo "WARNING: Some QZN tests failed"
RUN mkdir -p /app/store
EXPOSE 41841
WORKDIR /app/store
CMD ["/app/build/src/Qubic", "--ticking-delay", "1000"]