/*
 * Copyright (c) 2021 Sandeep Mistry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/apps/httpd.h"
#include "rmii_ethernet/netif.h"

#include "lwip/pbuf.h"
#include <stdio.h>
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"

#define ICMP_RATE_LIMIT 50

uint8_t timer_started = false;
uint8_t icmp_rate = 0;

// Alarm to detect ICMP Flood
//

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     // printf("Timer %d fired!\n", (int) id);
//     if (icmp_rate > ICMP_RATE_LIMIT){
//         printf("ICMP FLOOOOD");
//     }
//     icmp_rate = 0;
//     timer_started = false;
//     // Can return a value here in us to fire in the future
//     return 0;
// }

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // printf("Timer %d fired!\n", (int) id);
    if (icmp_rate > ICMP_RATE_LIMIT){
        printf("ICMP FLOOOOD");
    }
    icmp_rate = 0;
    timer_started = false;
    // Can return a value here in us to fire in the future
    return 0;
}

// This will be called whenever an ICMP packet is received
//
static u8_t icmp_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr) {
    printf("Received ICMP packet\n");

    // Print the length of the received packet
    //
    printf("Packet Length: %d bytes\n", p->tot_len);

    // Iterate through the pbuf chain and print each byte
    //
    struct pbuf *q;
    for (q = p; q != NULL; q = q->next) {
        for (int i = 0; i < q->len; i++) {
            printf("%02X ", ((u8_t *)q->payload)[i]);
            // You can also print characters if needed:
            // printf("%c", ((u8_t *)q->payload)[i]);
        }
    pbuf_free(q);
    }

    // Extract the ICMP type from byte 21 (offset 20) in the packet
    //
    u8_t icmp_type = ((u8_t *)p->payload)[20];
    printf("ICMP type: %d\n", icmp_type);
    printf("\n");

    if (icmp_type = ICMP_ECHO){
        printf("Hit");
        // icmp_rate++;
        if (!timer_started) {
            add_alarm_in_ms(1000, alarm_callback, NULL, false);
            timer_started = true;
        }
        else if (timer_started) {
            icmp_rate++;
        }
    }

    return 1;
}

// Setup function to listen for ICMP packets
//
void setup_icmp_listener(void) {
    // Create a new raw protocol control block (PCB) for ICMP protocol
    //
    struct raw_pcb *raw = raw_new(IP_PROTO_ICMP);
    
    // Register the `icmp_recv` function as the callback for ICMP packet reception
    // The `NULL` argument is used to indicate that no additional user data is needed for the callback
    //
    raw_recv(raw, icmp_recv, NULL);
    
    // Bind the raw PCB to listen on all available network interfaces
    //
    raw_bind(raw, IP_ADDR_ANY);
    
    // Print a message to indicate that the ICMP listener has been set up successfully
    //
    printf("ICMP listener set up\n");
}

void netif_link_callback(struct netif *netif)
{
    printf("netif link status changed %s\n", netif_is_link_up(netif) ? "up" : "down");
}

void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

#include <stdio.h>
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

int main() {

    // LWIP network interface
    struct netif netif;

    struct netif_rmii_ethernet_config netif_config = {
        pio0, // PIO:            0
        0,    // pio SM:         0 and 1
        6,    // rx pin start:   6, 7, 8    => RX0, RX1, CRS
        10,   // tx pin start:   10, 11, 12 => TX0, TX1, TX-EN
        14,   // mdio pin start: 14, 15   => ?MDIO, MDC
        NULL, // MAC address (optional - NULL generates one based on flash id) 
    };

    // change the system clock to use the RMII reference clock from pin 20
    clock_configure_gpin(clk_sys, 20, 50 * MHZ, 50 * MHZ);
    sleep_ms(1000);

    // initialize stdio after the clock change
    stdio_init_all();
    sleep_ms(5000);
    
    printf("pico rmii ethernet - httpd\n");

    // initilize LWIP in NO SYS mode
    lwip_init();

    // initialize the PIO base RMII Ethernet network interface
    netif_rmii_ethernet_init(&netif, &netif_config);
    
    // assign callbacks for link and status
    netif_set_link_callback(&netif, netif_link_callback);
    netif_set_status_callback(&netif, netif_status_callback);

    // set the default interface and bring it up
    netif_set_default(&netif);
    netif_set_up(&netif);

    #include "lwip/opt.h"
    #include "lwip/init.h"
    #include "lwip/netif.h"
    #include "lwip/ip4_addr.h"
    #include "lwip/tcpip.h"
    #include "netif/ethernet.h"

    ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&ipaddr, 192,168,1,100);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,1,1);

    // Apply the configuration
    //
    netif_set_addr(&netif, &ipaddr , &netmask, &gw);
    netif_set_up(&netif);

    //  No longer required as we static assign IP
    //
    //  dhcp_start(&netif);
    //  httpd_init();

    setup_icmp_listener();

    // setup core 1 to monitor the RMII ethernet interface, core0 is free
    //
    multicore_launch_core1(netif_rmii_ethernet_loop);


    while (1) {
        tight_loop_contents();
    }
    return 0;
}