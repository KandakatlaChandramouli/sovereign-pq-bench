#!/usr/bin/env bash
set -e

echo "======================================"
echo " Sovereign PQ Benchmark Bootstrap"
echo "======================================"

############################################
# Root CMakeLists.txt
############################################

cat > CMakeLists.txt <<'CMAKE'
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

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
        -O3
        -march=native
        -DNDEBUG
    )
endif()

include_directories(include)

add_executable(
    sovereign_bench
    benchmarks/main.cpp
)
CMAKE

############################################
# Main
############################################

cat > benchmarks/main.cpp <<'CPP'
#include <iostream>

int main()
{
    std::cout << "=====================================\n";
    std::cout << " Sovereign PQ Benchmark Framework\n";
    std::cout << " Version 0.1.0\n";
    std::cout << "=====================================\n\n";

    std::cout << "Framework bootstrapped successfully.\n";

    return 0;
}
CPP

echo ""
echo "Stage 1 Bootstrap Complete!"
