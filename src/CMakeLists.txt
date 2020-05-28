add_subdirectory(cpu/)

add_executable(graphcutseg
    main.cc)

target_link_libraries(graphcutseg
    PUBLIC SDL2::SDL2 
    PUBLIC CLI11::CLI11
    PRIVATE cpu)

target_include_directories(graphcutseg
    PUBLIC ${CMAKE_SOURCE_DIR}/include/cpu
    PUBLIC ${SDL2_INCLUDE_DIRS})

add_custom_command(TARGET graphcutseg
    POST_BUILD
    COMMAND cp graphcutseg ..)
