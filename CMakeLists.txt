cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(pong C CXX ASM)

pico_sdk_init()

add_executable(pong
        main.cpp
    )

pico_enable_stdio_usb(pong 1)
pico_enable_stdio_uart(pong 0)

target_link_libraries(pong pico_stdlib hardware_spi) #hardware_spi hardware_i2c
pico_add_extra_outputs(pong)