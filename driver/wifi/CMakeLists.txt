cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME pico20Proj)
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

pico_sdk_init()
project(project_name)

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)


add_executable(${PROGRAM_NAME}
        picow_access_point.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        )
target_include_directories(${PROGRAM_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        )
target_link_libraries(${PROGRAM_NAME}
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

# Ucomment when using NEXMON driver
target_compile_definitions(${PROGRAM_NAME} PRIVATE
         CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE="${NEXMON_ROOT}/patches/bcm43439a0/7_95_49_2271bb6/nexmon/w43439A0_7_95_49_00_combined.h"
         CYW43_WIFI_NVRAM_INCLUDE_FILE="${PICO_NEXMON_PATH}/cyw43-driver/firmware/wifi_nvram_43439.h"
         CYW43_ENABLE_BLUETOOTH=0
         )

pico_add_extra_outputs(${PROGRAM_NAME})
pico_enable_stdio_usb(${PROGRAM_NAME} TRUE)
pico_enable_stdio_uart(${PROGRAM_NAME} TRUE)