# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(DisplayC C CXX ASM)

pico_sdk_init()

add_executable(DisplayC DisplayC.c inc/ssd1306.c)


pico_set_program_name(DisplayC "DisplayC")
pico_set_program_version(DisplayC "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(DisplayC 0)
pico_enable_stdio_usb(DisplayC 1)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(DisplayC ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(DisplayC PRIVATE DisplayC.c inc/ssd1306.c)

target_link_libraries(DisplayC PRIVATE pico_stdlib hardware_i2c hardware_pio)

pico_add_extra_outputs(DisplayC)
