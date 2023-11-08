add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(sdcard sd_card.c)

# pull in common dependencies
target_link_libraries(sdcard pico_stdlib FatFs_SPI)

# enable usb output, enable uart output
pico_enable_stdio_usb(sdcard 1)
pico_enable_stdio_uart(sdcard 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(sdcard)

# add url via pico_set_program_url
example_auto_set_url(sdcard)