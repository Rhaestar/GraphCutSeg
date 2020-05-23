include_directories(${SDL2_INCLUDE_DIRS})

add_executable(graphcutseg
    main.cc)

target_link_libraries(graphcutseg SDL2::SDL2 CLI11::CLI11)

add_custom_command(TARGET graphcutseg
    POST_BUILD
    COMMAND cp graphcutseg ..)
