cmake_minimum_required(VERSION 3.20)
project(sovereign_pq_bench
    VERSION 1.0.0
    DESCRIPTION "Quantifying the Cost of Sovereignty: PQ Migration Benchmarking Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

find_package(OpenSSL REQUIRED)
find_package(benchmark REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

find_package(liboqs QUIET)
if(NOT liboqs_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        liboqs
        GIT_REPOSITORY https://github.com/open-quantum-safe/liboqs.git
        GIT_TAG 0.10.0
        GIT_SHALLOW TRUE
    )
    set(OQS_MINIMAL_BUILD ON CACHE BOOL "Minimal build")
    set(OQS_USE_OPENSSL OFF CACHE BOOL "No OpenSSL in liboqs")
    FetchContent_MakeAvailable(liboqs)
endif()

add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wnon-virtual-dtor -Wold-style-cast
    -Wcast-align -Wunused -Woverloaded-virtual
    -Wconversion -Wsign-conversion -Wnull-dereference
    -Wdouble-promotion -Wformat=2
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(sovereign_engine STATIC
    src/crypto/signature_scheme.cpp
    src/crypto/ecdsa_engine.cpp
    src/crypto/ml_dsa_engine.cpp
    src/metrics/metrics_collector.cpp
    src/metrics/system_info.cpp
    src/metrics/benchmark_result.cpp
    src/metrics/csv_exporter.cpp
    src/metrics/json_exporter.cpp
    src/metrics/markdown_exporter.cpp
    src/utils/random_generator.cpp
    src/utils/timer.cpp
)

target_include_directories(sovereign_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sovereign_engine
    PUBLIC
        OpenSSL::Crypto
        Threads::Threads
        oqs
    PRIVATE
        benchmark::benchmark
)

target_compile_features(sovereign_engine PUBLIC cxx_std_20)

add_executable(sovereign_cli apps/main.cpp)
target_link_libraries(sovereign_cli PRIVATE sovereign_engine)

if(BUILD_BENCHMARKS)
    add_executable(sovereign_bench benchmarks/bench_main.cpp)
    target_link_libraries(sovereign_bench PRIVATE sovereign_engine benchmark::benchmark)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS sovereign_engine ARCHIVE DESTINATION lib)
install(DIRECTORY include/sovereign DESTINATION include)
