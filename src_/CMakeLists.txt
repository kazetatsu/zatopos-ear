# based on pico-examples

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(zatoichi_worker C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# add_executable(main usb_multicore3.c dev_lowlevel.c micarr.c)
add_executable(main usb_multicore4.c micarr.c)
target_link_libraries(main PRIVATE
    pico_stdlib pico_multicore pico_sync
    hardware_resets hardware_irq hardware_adc hardware_gpio)

# add_executable(main main.c dev_lowlevel.c)
# target_link_libraries(main PRIVATE
#     pico_stdlib pico_multicore pico_sync
#     hardware_gpio hardware_spi hardware_timer hardware_resets hardware_irq)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)
