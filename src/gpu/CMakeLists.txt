add_library(gpu STATIC
    histogram.cc)

target_compile_features(gpu PUBLIC cxx_std_14)

target_sources(gpu PRIVATE gpuimplem.cu)

set_target_properties(gpu PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries(gpu
    PUBLIC SDL2::SDL2)

target_include_directories(gpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/gpu)
