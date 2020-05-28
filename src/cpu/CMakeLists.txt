add_library(cpu STATIC
    histogram.cc
    implem.cc)

target_link_libraries(cpu
    PUBLIC SDL2::SDL2)

target_include_directories(cpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/cpu)
