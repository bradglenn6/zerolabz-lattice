cmake_minimum_required(VERSION 3.22)
project(ZeroLabzLatticeCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lattice_core INTERFACE)
target_include_directories(lattice_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(lattice_core INTERFACE /O2 /arch:AVX2)
else()
    target_compile_options(lattice_core INTERFACE -O3 -mavx2 -mfma)
endif()

option(BUILD_BENCHMARKS "Build cycle-accurate hardware benchmarks" ON)
if(BUILD_BENCHMARKS)
    enable_testing()
    add_subdirectory(benchmarks)
endif()
