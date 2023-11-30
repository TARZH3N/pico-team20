cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME pico20Proj)
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(project_name)

pico_sdk_init()

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(${PROGRAM_NAME}
    main.c
)

target_include_directories(${PROGRAM_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Add pico_stdlib library, add more if used
target_link_libraries(${PROGRAM_NAME}
    pico_stdlib 
    hardware_pwm 
    hardware_adc 
    hardware_timer 
    hardware_irq 
    hardware_clocks 
    hardware_spi 
    hardware_gpio  
    hardware_pio
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    FatFs_SPI
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROGRAM_NAME} 1)
pico_enable_stdio_uart(${PROGRAM_NAME} 0)

# Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(${PROGRAM_NAME})