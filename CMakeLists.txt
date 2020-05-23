cmake_minimum_required (VERSION 3.14)

include(FetchContent)

project(GraphCutSeg
    VERSION 1.0
    LANGUAGES CUDA CXX)

FetchContent_Declare(
  CLI11
  URL https://github.com/CLIUtils/CLI11/archive/v1.8.0.tar.gz
  )

FetchContent_MakeAvailable(CLI11)

find_package(CUDA REQUIRED)
find_package(SDL2 REQUIRED)

add_compile_options(-Wall -Wextra -Werror -std=c++17 -g -pedantic)

add_subdirectory(src/)
