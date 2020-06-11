add_subdirectory(cpu/)
add_subdirectory(gpu/)

add_executable(graphcutseg
    main.cc)

target_link_libraries(graphcutseg
    PUBLIC SDL2::SDL2 
    PUBLIC CLI11::CLI11
    PRIVATE cpu
    PRIVATE gpu)
#    PUBLIC -fsanitize=address)

target_include_directories(graphcutseg
    PUBLIC ${CMAKE_SOURCE_DIR}/include/cpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/gpu
    PUBLIC ${SDL2_INCLUDE_DIRS})

add_custom_command(TARGET graphcutseg
    POST_BUILD
    COMMAND cp graphcutseg ..)

add_executable(bench bench.cpp)

target_link_libraries(bench 
    PUBLIC SDL2::SDL2
    PRIVATE benchmark
    PRIVATE cpu
    PRIVATE gpu)

target_include_directories(bench
    PUBLIC ${CMAKE_SOURCE_DIR}/include/cpu
    PUBLIC ${CMAKE_SOURCE_DIR}/include/gpu
    PUBLIC ${SDL2_INCLUDE_DIRS})

add_custom_command(TARGET bench
    POST_BUILD
    COMMAND cp bench ..)

