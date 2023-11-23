#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"volt","temp","led"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  switch (iIndex) {
  case 0: // volt
    {
      //Format the voltage to be printed out
      const float voltage = adc_read() * 3.3f / (1 << 12);
      printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
    }
    break;
  case 1: // temp
    {
    //Format the temperature to be printed out
    const float voltage = adc_read() * 3.3f / (1 << 12);
    const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
    printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
    }
    break;
  case 2: // led
    { //Format the LED Status to be printed out
      bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
      if(led_status == true){
        //If LED is ON
        printed = snprintf(pcInsert, iInsertLen, "ON");
      }
      else{
        //If LED is OFF
        printed = snprintf(pcInsert, iInsertLen, "OFF");
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