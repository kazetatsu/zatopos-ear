# based on pico-examples

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(../pico_sdk_import.cmake)

project(zatoichi_test_spi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(main main.c)

target_link_libraries(main
    pico_stdlib hardware_gpio hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)
