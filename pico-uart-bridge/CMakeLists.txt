cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_uart_bridge C CXX ASM)
pico_sdk_init()

add_executable(pico_uart_bridge main.c usb_descriptors.c)
pico_generate_pio_header(pico_uart_bridge ${CMAKE_CURRENT_LIST_DIR}/uart.pio)
target_include_directories(pico_uart_bridge PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(pico_uart_bridge pico_stdlib hardware_pio tinyusb_device)

pico_enable_stdio_uart(pico_uart_bridge 0)
pico_enable_stdio_usb(pico_uart_bridge 0)
pico_add_extra_outputs(pico_uart_bridge)
