#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"

//WIFI CONNECTION DETAILS
const char WIFI_SSID[] = "Seth's Galaxy S21 FE 5G";
const char WIFI_PASSWORD[] = "wvme6694";


//Main Function
int main(){
    stdio_init_all();

    //Initialise the WIFI chip
    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    //Connecting to Wifi Network as set up above
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)){
        printf("Connecting...\n");
    }
    
    printf("Connected!\n");

    //Enable server
    httpd_init();
    printf("Server is initialised!");

    // Configure SSI and CGI handler
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    
    // Infinite loop to keep the server running
    while(1);
}