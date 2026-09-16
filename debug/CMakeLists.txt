set(PROJECT_NAME Debug)

cmake_minimum_required(VERSION 3.13...3.27)
include(pico_sdk_import.cmake)
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME})
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

add_subdirectory(.. a2pico)

target_sources(${PROJECT_NAME} PRIVATE
        board.c
        main.c
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        a2pico
        pico_multicore
        pico_stdlib
        )
