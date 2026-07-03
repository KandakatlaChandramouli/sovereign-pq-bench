cmake_minimum_required(VERSION 3.20)
project(sovereign_pq_bench VERSION 2.0.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(liboqs
    GIT_REPOSITORY https://github.com/open-quantum-safe/liboqs.git
    GIT_TAG 0.10.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(liboqs)

add_compile_options(-Wall -Wextra -Wpedantic)

set(SOURCES
    src/crypto/signature_scheme.cpp
    src/crypto/ecdsa_engine.cpp
    src/crypto/ed25519_engine.cpp
    src/crypto/ml_dsa_engine.cpp
    src/crypto/pq_engine.cpp
    src/metrics/metrics_collector.cpp
    src/metrics/system_info.cpp
    src/metrics/benchmark_result.cpp
    src/metrics/csv_exporter.cpp
    src/metrics/json_exporter.cpp
    src/metrics/markdown_exporter.cpp
    src/metrics/statistical_analysis.cpp
    src/metrics/energy_meter.cpp
    src/utils/random_generator.cpp
    src/utils/timer.cpp
)

add_library(sovereign_engine STATIC ${SOURCES})
target_include_directories(sovereign_engine
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${liboqs_BINARY_DIR}/include
)
target_link_libraries(sovereign_engine PUBLIC OpenSSL::Crypto Threads::Threads oqs)
target_compile_features(sovereign_engine PUBLIC cxx_std_20)

add_executable(sovereign_cli apps/main.cpp)
target_link_libraries(sovereign_cli PRIVATE sovereign_engine)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
