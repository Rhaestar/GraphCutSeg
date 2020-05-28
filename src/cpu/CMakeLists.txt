add_library(cpu STATIC
    histogram.cc)

target_include_directories(cpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/cpu)
