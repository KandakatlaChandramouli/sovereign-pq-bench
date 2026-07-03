cmake_minimum_required(VERSION 3.20)

project(
    SovereignPQBench
    VERSION 0.1.0
    DESCRIPTION "Reproducible Cryptographic Benchmarking Framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -DNDEBUG)
endif()

find_package(benchmark REQUIRED)

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
    benchmark::benchmark_main
)
