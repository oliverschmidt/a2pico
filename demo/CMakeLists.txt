set(PROJECT_NAME demo)

cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME})
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

add_subdirectory(.. a2pico)

set_source_files_properties(incbin.S OBJECT_DEPENDS firmware.rom)
add_custom_command(
        COMMAND cl65 ${CMAKE_CURRENT_SOURCE_DIR}/firmware.S
                  -C ${CMAKE_CURRENT_SOURCE_DIR}/firmware.cfg
                  -o ${CMAKE_CURRENT_BINARY_DIR}/firmware.rom
        MAIN_DEPENDENCY firmware.S
        DEPENDS firmware.cfg
        OUTPUT firmware.rom
        VERBATIM
        )

target_sources(${PROJECT_NAME} PRIVATE
        main.c
        board.c
        incbin.S
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_multicore
        a2pico
        )
