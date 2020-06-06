add_library(gpu STATIC
    histogram.cc
    gpuimplem.cc)

target_link_libraries(gpu
    PUBLIC SDL2::SDL2)

target_include_directories(gpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/gpu)
