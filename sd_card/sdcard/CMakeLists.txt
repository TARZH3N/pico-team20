add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
set(PROGRAM_NAME sdcarddriver)
project(sdcarddriver)
add_executable(sdcarddriver sd_card.c)

# pull in common dependencies
target_link_libraries(sdcarddriver pico_stdlib FatFs_SPI)

# enable usb output, enable uart output
pico_enable_stdio_usb(sdcarddriver 1)
pico_enable_stdio_uart(sdcarddriver 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(sdcarddriver)

# add url via pico_set_program_url
#example_auto_set_url(sdcarddriver)