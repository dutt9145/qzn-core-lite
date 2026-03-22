FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -o Acquire::AllowInsecureRepositories=true -o Acquire::AllowDowngradeToInsecureRepositories=true || apt-get update --allow-unauthenticated || true
RUN apt-get install -y --allow-unauthenticated \
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
RUN sed -i '/TestExample/d' /app/src/contract_core/contract_def.h || true
RUN sed -i '/TESTEX/d' /app/src/contract_core/contract_def.h || true
RUN mkdir -p /app/build && cd /app/build && cmake .. \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-mavx2 -mno-avx512f -DTESTEXA_CONTRACT_INDEX=32" \
      -DCMAKE_C_FLAGS="-mavx2 -mno-avx512f -DTESTEXA_CONTRACT_INDEX=32" \
      -DBUILD_TESTS=OFF
RUN cd /app/build && make -j$(nproc) fmt trantor drogon platform_common platform_efi
COPY contracts/ /qzn/contracts/
COPY test/      /qzn/test/
COPY setup.sh   /qzn/setup.sh
COPY patch.py   /qzn/patch.py
COPY fix_headers.py /qzn/fix_headers.py
RUN chmod +x /qzn/setup.sh && /qzn/setup.sh && cp /qzn/test/contract_globals.cpp /app/test/ && cp /qzn/test/concurrency_impl.cpp /app/test/
RUN python3 /qzn/fix_headers.py
RUN sed -i '1s/^/#pragma once\n/' /app/src/contracts/QZN_TournamentEngine_PAO.h
RUN sed -i '1s/^/#pragma once\n/' /app/src/contracts/QZN_TreasuryVault_PAO.h
RUN echo '#include "contract_tester.h"' >> /app/test/contract_testing.h
RUN sed -i '1s/^/#ifndef TESTEXA_CONTRACT_INDEX\n#define TESTEXA_CONTRACT_INDEX 32\n#endif\n/' /app/src/contract_core/contract_exec.h
RUN for f in /app/src/contracts/QZN_*.h; do grep -q "pragma once" "$f" || sed -i '1s/^/#pragma once\n/' "$f"; done
RUN printf 'cmake_minimum_required(VERSION 3.15)\nproject(qzn_tests CXX C)\nset(CMAKE_CXX_STANDARD 20)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\ninclude(FetchContent)\nFetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.16.0)\nset(gtest_force_shared_crt ON CACHE BOOL "" FORCE)\nFetchContent_MakeAvailable(googletest)\nadd_executable(qzn_tests\n    contract_qzn_token.cpp\n    contract_qzn_gamecabinet.cpp\n    contract_qzn_portal.cpp\n    contract_qzn_rewardrouter.cpp\n    contract_qzn_tournamentengine.cpp\n    contract_qzn_treasuryvault.cpp\n    contract_globals.cpp\n    concurrency_impl.cpp\n    spectrum.cpp\n    stdlib_impl.cpp\n    assets.cpp\n    platform.cpp\n)\ntarget_include_directories(qzn_tests PRIVATE\n    ${CMAKE_CURRENT_SOURCE_DIR}\n    ${CMAKE_CURRENT_SOURCE_DIR}/../src\n    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/platform_common\n    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/platform_os\n    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/platform_efi\n    ${CMAKE_CURRENT_SOURCE_DIR}/..\n)\ntarget_compile_options(qzn_tests PRIVATE -mrdrnd -mbmi -mlzcnt -fshort-wchar -w -Wno-error -mavx2 -DTESTEXA_CONTRACT_INDEX=32)\ntarget_link_options(qzn_tests PRIVATE -Wl,--no-relax)\ntarget_link_libraries(qzn_tests PRIVATE GTest::gtest_main platform_common platform_efi platform_os)\ninclude(GoogleTest)\ngtest_discover_tests(qzn_tests)\n' > /app/test/CMakeLists.txt
RUN cd /app/build && cmake .. \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-mavx2 -mno-avx512f -DTESTEXA_CONTRACT_INDEX=32" \
      -DCMAKE_C_FLAGS="-mavx2 -mno-avx512f -DTESTEXA_CONTRACT_INDEX=32" \
      -DBUILD_TESTS=ON
RUN grep -rn "SystemProcedureID\|BEGIN_EPOCH\|END_TICK\|callSystemProcedure" /app/src/contract_core/contract_exec.h | head -20
RUN cd /app/build && make -j$(nproc) platform_os qzn_tests
RUN /app/build/test/qzn_tests
RUN cd /app/build && make -j$(nproc) Qubic
RUN mkdir -p /app/store
EXPOSE 41841
WORKDIR /app/store
CMD ["/app/build/src/Qubic", "--ticking-delay", "1000"]
