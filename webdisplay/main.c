#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"
#include <stdio.h>
#include "sd_card.h"
#include "ff.h"

// WIFI Information
//
const char WIFI_SSID[] = "zhenyu";
const char WIFI_PASSWORD[] = "qwerty123";

// Serial Monitor Troubleshooting
//
void list_files(const char *path) {
    FRESULT fr;
    DIR dir;
    static FILINFO fno;

    fr = f_opendir(&dir, path); // Open directory
    if (fr != FR_OK) {
        printf("ERROR: Could not open directory (%d)\r\n", fr);
        return;
    }

    while (true) {
        fr = f_readdir(&dir, &fno); // Read a directory item
        if (fr != FR_OK || fno.fname[0] == 0) {
            break; // Break on error or end of directory
        }
        if (fno.fattrib & AM_DIR) { // Check if it's a directory
            printf("Directory: %s\r\n", fno.fname);
        } else {
            printf("File: %s\r\n", fno.fname);
        }
    }
}

// Initialise the SSI handler
//
void ssi_init() {

  // Initialise ADC (internal pin)
  //
  adc_init();

  // Enable Temperature
  //
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);

  // Enable http handler
  //
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

// Main Function
//
int main(){
    FRESULT fr;
    DIR dir;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    static FILINFO fno;

    stdio_init_all();

    // Initialise CYW43, enable it in station mode
    //
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    // Initialize SD card
    //
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount drive
    //
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    printf("SD DRIVER INITIALIZED!\n");
    list_files("0:"); 
    
    // Establishing WIFI Connection
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)){
        printf("Connecting...\n");
    }
    
    printf("Connected!\n");
    
    // Initialize HTTP Server
    //
    httpd_init();
    printf("Server is initialised!\n");

    // Configure SSI and CGI handler
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    
    // Infinite loop to keep the server running
    while(1);
}
