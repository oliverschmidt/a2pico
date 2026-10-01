set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards PARENT_SCOPE)

function(a2pico_init)
        if (PICO_RP2350)
                set(A2PICO "2")
        endif ()

        add_library(a2pico INTERFACE)

        target_include_directories(a2pico INTERFACE ${CMAKE_CURRENT_FUNCTION_LIST_DIR})

        target_sources(a2pico INTERFACE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/a2pico${A2PICO}.c)

        target_link_libraries(a2pico INTERFACE hardware_adc hardware_pio pico_stdlib)

        pico_generate_pio_header(a2pico ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/a2pico${A2PICO}.pio)
endfunction()

cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL a2pico_init)
