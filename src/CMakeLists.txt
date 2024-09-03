# based on pico-examples

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(zatoichi_worker C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(main_usb)
add_executable(main_serial)

pico_generate_pio_header(main_usb ${CMAKE_CURRENT_LIST_DIR}/mcp3008.pio)
pico_generate_pio_header(main_usb ${CMAKE_CURRENT_LIST_DIR}/mcp3008_6ch_full.pio)

pico_generate_pio_header(main_serial ${CMAKE_CURRENT_LIST_DIR}/mcp3008.pio)
pico_generate_pio_header(main_serial ${CMAKE_CURRENT_LIST_DIR}/mcp3008_6ch_full.pio)

target_sources(main_usb    PRIVATE main_usb.c    sound.c mic.c)
target_sources(main_serial PRIVATE main_serial.c sound.c mic.c)

target_link_libraries(main_usb
    pico_stdlib pico_multicore pico_sync pico_time
    hardware_dma hardware_gpio hardware_pio)
target_link_libraries(main_serial
    pico_stdlib pico_time
    hardware_dma hardware_gpio hardware_pio)

# enable usb output, disable uart output
pico_enable_stdio_usb(main_usb 0)
pico_enable_stdio_uart(main_usb 1)

pico_enable_stdio_usb(main_serial 1)
pico_enable_stdio_uart(main_serial 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(main_usb)
pico_add_extra_outputs(main_serial)
