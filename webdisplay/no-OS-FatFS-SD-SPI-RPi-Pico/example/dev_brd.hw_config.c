/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/
/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
In general, this should correspond to the (zero origin) array index.
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/

#include <string.h>
//
#include "my_debug.h"
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */

/* 
See https://docs.google.com/spreadsheets/d/1BrzLWTyifongf_VQCc2IpJqXWtsrjmG7KnIbSBy-CPU/edit?usp=sharing,
tab "Monster", for pin assignments assumed in this configuration file.
*/

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi0,  // SPI component
        .sck_gpio = 10,  // GPIO number (not Pico pin number)
        .mosi_gpio = 12,
        .miso_gpio = 11,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        // .baud_rate = 25 * 1000 * 1000,  // Actual frequency: 20833333.
        .baud_rate = 12 * 1000 * 1000,  // Actual frequency: 10416666.
       // .baud_rate = 1 * 1000 * 1000,
    },
    {
        .hw_inst = spi1,  // SPI component
        .sck_gpio = 10,  // GPIO number (not Pico pin number)
        .mosi_gpio = 12,
        .miso_gpio = 11,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        // .baud_rate = 25 * 1000 * 1000,  // Actual frequency: 20833333.
        .baud_rate = 12 * 1000 * 1000,  // Actual frequency: 10416666.
    }
   };

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {        // Socket sd0 over SPI
        .pcName = "0:",  // Name used to mount device
        .spi = &spis[0],  // Pointer to the SPI driving this card
        .ss_gpio = 13,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .card_detect_gpio = 15,
        .card_detected_true = 1,
    },
    {        // Socket sd1
        .pcName = "1:",  // Name used to mount device
        .spi = &spis[1],  // Pointer to the SPI driving this card
        .ss_gpio = 13,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .card_detect_gpio = 15,
        .card_detected_true = 1,
    },
    {        // Socket sd2
        .pcName = "2:",  // Name used to mount device
        .spi = &spis[1],  // Pointer to the SPI driving this card
        .ss_gpio = 13,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .card_detect_gpio = 15,
        .card_detected_true = 1,
    },
    };

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= spi_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */
