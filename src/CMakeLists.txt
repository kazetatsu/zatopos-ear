# based on pico-examples

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(zatoichi_worker C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(main)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/mcp3008.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/mcp3008_6ch_full.pio)

target_sources(main PRIVATE main.c mic.c)
# target_sources(main PRIVATE mic_test.c mic.c)

target_link_libraries(main
    pico_stdlib pico_multicore pico_sync pico_time
    hardware_dma hardware_gpio hardware_pio)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)
