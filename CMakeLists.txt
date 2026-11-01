cmake_minimum_required(VERSION 3.22)
project(ZeroLabzLatticeCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lattice_core INTERFACE)
target_include_directories(lattice_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(lattice_core INTERFACE /O2 /Oi /Ot /arch:AVX512)
else()
    target_compile_options(lattice_core INTERFACE -O3 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mfma)
endif()

enable_testing()

option(BUILD_BENCHMARKS "Build cycle-accurate hardware benchmarks" ON)
if(BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
endif()

option(BUILD_TESTS "Build manifold and NIST PQC Known Answer Tests" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()