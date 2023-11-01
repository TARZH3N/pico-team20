add_executable(WiFi_AP_Pico
        main.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        )

target_include_directories(WiFi_AP_Pico PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        )

target_link_libraries(WiFi_AP_Pico
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

pico_add_extra_outputs(WiFi_AP_Pico)

# enable usb output, disable uart output
pico_enable_stdio_usb(WiFi_AP_Pico 1)
pico_enable_stdio_uart(WiFi_AP_Pico 0)