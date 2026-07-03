cmake_minimum_required(VERSION 3.20)

project(
    SovereignPQBench
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(benchmark REQUIRED)
find_library(OQS_LIBRARY oqs REQUIRED)

add_subdirectory(src/core)

add_executable(
    sovereign_bench
    benchmarks/main.cpp
)

target_include_directories(
    sovereign_bench
    PRIVATE
        include
)

target_link_libraries(
    sovereign_bench
    PRIVATE
        spqb_core
        benchmark::benchmark
        benchmark::benchmark_main
        OpenSSL::Crypto
        ${OQS_LIBRARY}
)
