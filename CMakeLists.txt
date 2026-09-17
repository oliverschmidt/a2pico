add_library(a2pico INTERFACE)

if (PICO_RP2350)
        set(A2PICO "2")
endif ()

if (PICO_CYW43_SUPPORTED)
        target_compile_definitions(a2pico INTERFACE HAVE_ENBL)
endif ()

target_include_directories(a2pico INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(a2pico INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/a2pico${A2PICO}.c)

target_link_libraries(a2pico INTERFACE hardware_adc hardware_pio pico_stdlib)

pico_generate_pio_header(a2pico ${CMAKE_CURRENT_SOURCE_DIR}/a2pico${A2PICO}.pio)
