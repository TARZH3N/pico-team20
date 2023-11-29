#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
// #include "ssi.h"
#include "cgi.h"
#include <stdio.h>
#include "sd_card.h"
#include "ff.h"

// Define WIFI SSID
//
const char WIFI_SSID[] = "zhenyu";
const char WIFI_PASSWORD[] = "qwerty123";

void list_files(const char *path) {
    FRESULT fr;
    DIR dir;
    static FILINFO fno;

    // Open directory
    //
    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("ERROR: Could not open directory (%d)\r\n", fr);
        return;
    }

    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) {
            break;
        }
        if (fno.fattrib & AM_DIR) {
            printf("Directory: %s\r\n", fno.fname);
        } else {
            printf("File: %s\r\n", fno.fname);
        }
    }
}

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

// SSI tags - tag length limited to 8 bytes by default
//
const char * ssi_tags[] = {"volt","temp","led","icmpflood"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    size_t printed;
    switch (iIndex) {
        // Volt
        //
        case 0:
        {
            // Format the voltage to be printed out
            //
            const float voltage = adc_read() * 3.3f / (1 << 12);
            printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
        }
        break;
        // Temp
        //
        case 1:
        {
            // Format the temperature to be printed out
            //
            const float voltage = adc_read() * 3.3f / (1 << 12);
            const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
            printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
        }
        break;
        // Led
        //
        case 2:
        { 
            // Format the LED Status to be printed out
            //
            bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
            if (led_status == true) {
                // If LED is ON
                //
                printed = snprintf(pcInsert, iInsertLen, "ON");
            } else {
                // If LED is OFF
                //
                printed = snprintf(pcInsert, iInsertLen, "OFF");
            }
        }
        break;
        // Printing contents of i2c_data.txt to display
        // IP Address of Attacker conducting ICMP Flooding
        // File contents are stored in Hex, requires conversion to ASCII
        //
        case 3:
        {
            // Open "i2c_data.txt"
            //
            FIL fil;
            UINT br;
            FRESULT fr = f_open(&fil, "/i2c_data.txt", FA_READ);
            if (fr != FR_OK) {
                printed = snprintf(pcInsert, iInsertLen, "Error opening file");
            } else {
                // Buffer to hold two hex digits and null terminator
                //
                char hexBuffer[3] = {0};
                unsigned int hexValue;
                printed = 0;
                char ch;
                while (printed < iInsertLen - 1) {
                    // Read one byte (two hex digits)
                    //
                    fr = f_read(&fil, &ch, 1, &br);
                    if (fr != FR_OK || br == 0) {
                        break;
                    }
                    if (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\0') {
                        hexBuffer[0] = ch;
                        // Read the second hex digit
                        //
                        fr = f_read(&fil, &ch, 1, &br);
                        if (fr != FR_OK || br == 0) {
                            break;
                        }
                        hexBuffer[1] = ch;

                        // Convert hex to ASCII
                        //
                        sscanf(hexBuffer, "%02x", &hexValue);
                        if (hexValue != 0) {
                            pcInsert[printed++] = (char)hexValue;
                        }
                    }
                }
                // Null-terminate the string
                //
                pcInsert[printed] = '\0'; // Null-terminate the string
                f_close(&fil);
            }
        }
break;
        default:
            printed = 0;
            break;
    }

    return (u16_t)printed;
}


// Initialise the SSI handler
void ssi_init() {
  // Initialise ADC (internal pin)
  adc_init();

  //Enable Temperature
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);

  //Enable http hndlerD
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}



//#include "httpd.h"
//#include "httpd_ssi.h"

//Main Function
int main(){
    FRESULT fr;
    DIR dir;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    static FILINFO fno;

    stdio_init_all();

    // Initialise the WIFI chip
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

    // Verbose output
    //
    printf("SD DRIVER INITIALIZED!\n");
    list_files("0:"); 
    
    //Connecting to Wifi Network as set up above
    //
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)){
        printf("Connecting...\n");
    }
    
    printf("Connected!\n");
    
    //Enable server
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
