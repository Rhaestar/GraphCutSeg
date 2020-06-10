cmake_minimum_required (VERSION 3.14)

include(FetchContent)

project(GraphCutSeg
    VERSION 1.0
    LANGUAGES CUDA CXX)

FetchContent_Declare(
  GoogleBenchmark
  URL https://github.com/google/benchmark/archive/v1.4.1.tar.gz
  )

FetchContent_Declare(
  CLI11
  URL https://github.com/CLIUtils/CLI11/archive/v1.8.0.tar.gz
  )

if (NOT GoogleBenchmark_POPULATED)
  FetchContent_Populate(GoogleBenchmark)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "From Gtest")
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "From Gtest")
  add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR})
endif()

FetchContent_MakeAvailable(CLI11)

find_package(CUDA REQUIRED)
find_package(SDL2 REQUIRED)

add_compile_options(-std=c++14 -g -O3)

add_subdirectory(src/)
