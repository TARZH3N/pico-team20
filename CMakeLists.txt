# add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# add_executable(sdcard sd_card.c)

# # pull in common dependencies
# target_link_libraries(sdcard pico_stdlib FatFs_SPI)

# # enable usb output, enable uart output
# pico_enable_stdio_usb(sdcard 1)
# pico_enable_stdio_uart(sdcard 1)

# # create map/bin/hex file etc.
# pico_add_extra_outputs(sdcard)

# # add url via pico_set_program_url
# example_auto_set_url(sdcard)

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

if(NOT DEFINED NO_NEXMON)
if(NOT DEFINED NEXMON_ROOT)
if(DEFINED ENV{NEXMON_ROOT})
    set(NEXMON_ROOT $ENV{NEXMON_ROOT})
    message("Using NEXMON_ROOT from environment ('${NEXMON_ROOT}')")
else()
    message(FATAL_ERROR "NEXMON_ROOT or NO_NEXMON must be set.")
endif()
endif()
message("NEXMON_ROOT is ${NEXMON_ROOT}")
else()
message("Force no NEXMON (${NO_NEXMON}).")
endif()

project(pico_nexmon C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_NEXMON_PATH ${PROJECT_SOURCE_DIR})
set(PICO_BOARD "pico_w")
set(PICO_CYW43_DRIVER_PATH ${PROJECT_SOURCE_DIR}/cyw43-driver)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-unused-local-typedefs
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

if (PICO_CYW43_SUPPORTED)
    if (NOT TARGET pico_cyw43_arch)
        message("Pico W support required")
    else()
        # specific projects in subdirectories
	    add_subdirectory(driver/wifi)
    endif()
    add_subdirectory(sd_card)
endif()
