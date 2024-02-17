# minimum CMake version (2.8.3 due to CMAKE_CURRENT_LIST_DIR)
cmake_minimum_required(VERSION 2.8.3)

add_library(a2pico INTERFACE)

target_include_directories(a2pico INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_sources(a2pico INTERFACE ${CMAKE_CURRENT_LIST_DIR}/a2pico.c)

target_link_libraries(a2pico INTERFACE pico_stdlib hardware_pio)

pico_generate_pio_header(a2pico ${CMAKE_CURRENT_LIST_DIR}/a2pico.pio)
