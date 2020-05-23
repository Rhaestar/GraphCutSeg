cmake_minimum_required (VERSION 3.3)

project(GraphCutSeg
    VERSION 1.0
    LANGUAGES CUDA CXX)

find_package(CUDA REQUIRED)

add_compile_options(-Wall -Wextra -Werror -std=c++17 -g -pedantic)

add_subdirectory(src/)
