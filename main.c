// Include necessary libraries and header files
//
#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"
#include <stdio.h>
#include "sd_card.h"
#include "ff.h"

// WiFi Network Credentials
//
const char g_wifi_ssid[] = "CHANGE_YOUR_SSID_HERE";
const char g_wifi_password[] = "CHANGE_YOUR_CREDENTIALS_HERE";

// Function to list files in a directory
//
void list_files(const char *path) {

    FRESULT fr;
    DIR dir;
    static FILINFO fno;

    // Open the directory
    //
    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("ERROR: Could not open directory (%d)\r\n", fr);
        return;
    }

    // Read and print each item in the directory
    //
    for(;;) {

        // Read a directory item
        //
        fr = f_readdir(&dir, &fno);

        if (fr != FR_OK || fno.fname[0] == 0) {

            // Break on error or end of directory
            //
            break;
        }

        // Check if the item is a directory
        //
        if (fno.fattrib & AM_DIR) {
            printf("Directory: %s\r\n", fno.fname);
        } else {

            // Print the name of the file
            //
            printf("File: %s\r\n", fno.fname);
        }
    }
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

    // Initialize standard I/O
    //
    stdio_init_all();

    // Initialise the WiFi chip
    cyw43_arch_init();

    // Enable station mode on the WiFi chip
    //
    cyw43_arch_enable_sta_mode();

    // Initialize the SD card driver
    //
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount the SD card filesystem
    //
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    printf("SD DRIVER INITIALIZED!\n");

    // List files in the root directory of the SD card
    //
    list_files("0:");
        
    // Connecting to Wifi Network as set up above
    //
    while(cyw43_arch_wifi_connect_timeout_ms(g_wifi_ssid, g_wifi_password, CYW43_AUTH_WPA2_AES_PSK, 30000)){
        printf("Connecting...\n");
    }
    
    printf("Connected!\n");
    
    // Enable the HTTP server
    //
    httpd_init();
    printf("Server is initialised!\n");

    // Configure SSI and CGI handler
    // 
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    
    // Infinite loop to keep the server running
    //
    while(1);
}

/*** end of file ***/