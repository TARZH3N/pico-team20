#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "sd_card.h"
#include "ff.h"

// SSI tags - tag length limited to 8 bytes by default
//
const char * ssi_tags[] = {"volt","temp","led","flood"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    size_t printed;
    switch (iIndex) {
        case 0:
        {
            // Format the voltage to be printed out
            //
            const float voltage = adc_read() * 3.3f / (1 << 12);
            printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
        }
        break;
        case 1:
        {
            // Format the temperature to be printed out
            //
            const float voltage = adc_read() * 3.3f / (1 << 12);
            const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
            printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
        }
        break;
        case 2:
        { 
            // Format the LED Status to be printed out
            //
            bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
            if (led_status == true) {
                printed = snprintf(pcInsert, iInsertLen, "ON");
            } else {
                printed = snprintf(pcInsert, iInsertLen, "OFF");
            }
        }
        break;

        // Displaying packet contents in ASCII
        //
        case 3:
        {
            // Open the "i2c_data.txt" file which store IP Address of attackers
            //
            FIL fil;
            UINT br;
            FRESULT fr = f_open(&fil, "/i2c_data.txt", FA_READ);
            if (fr != FR_OK) {
                printed = snprintf(pcInsert, iInsertLen, "Error opening file");
            } 
            else {
                // Buffer to hold two hex digits and null terminator
                //
                char hexBuffer[3] = {0};
                unsigned int hexValue;
                printed = 0;
                char ch;
                while (printed < iInsertLen - 1) 
                {
                    // Read one byte (two hex digits)
                    //
                    fr = f_read(&fil, &ch, 1, &br);
                    if (fr != FR_OK || br == 0) {
                        break;
                    }
                    // Skip null and whitespace characters
                    //
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
                        if (hexValue != 0) { // Ignore null characters
                            pcInsert[printed++] = (char)hexValue;
                        }
                    }
                }
                // Null-terminate the string
                //
                pcInsert[printed] = '\0';
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