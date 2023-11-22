#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/apps/httpd.h"
#include "rmii_ethernet/netif.h"

#include "lwip/pbuf.h"
#include <stdio.h>

#include "lwip/inet_chksum.h"
#include "lwip/icmp.h"
#include "lwip/ip4.h"


void print_packet(struct pbuf* p) {
    // Traverse through the pbuf chain
    while (p != NULL) { 
        for (u16_t i = 0; i < p->len; i++) {
            printf("%02x ", ((unsigned char*)p->payload)[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        p = p->next; // Move to the next pbuf in the chain
    }
    printf("\n");
}


// Used this to test internet connectivity, by using it to ping to Google DNS Server
//
void send_icmp_ping(struct netif *netif, const ip4_addr_t *dest_ip) {

    // Allocate pbuf for ICMP packet
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct icmp_echo_hdr), PBUF_RAM);
    if (!p) {
        printf("Failed to allocate pbuf for ICMP\n");
        return;
    }

    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)p->payload;

    // Fill in the ICMP header
    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id = htons(0xABCD); // arbitrary ID
    iecho->seqno = htons(1);   // sequence number

    // Calculate the ICMP checksum
    iecho->chksum = inet_chksum(iecho, p->len);

    // Send the packet
    // ip4_output_if(p, NULL, dest_ip, &netif->ip_addr, IP_PROTO_ICMP);
    ip4_output_if(p, &netif->ip_addr, dest_ip, IP_DEFAULT_TTL, 0, IP_PROTO_ICMP, netif);
    //print_packet(p);
    pbuf_free(p);
    printf("ICMP Ping sent\n");
}


#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"

// Define a basic ICMP header structure.
// This will be called whenever an ICMP packet is received.

// Check for ICMP Echo Reply type
// Click on the link below to view example of packet received
// It detects at the wrong offset, thus we might have to manually specify that we have to look at offset 21 to detect pings
// https://hpd.gasmi.net/?data=4500003C6A56000080014CB5C0A80101C0A8016408004D4B000100106162636465666768696A6B6C6D6E6F7071727374757677616263646566676869&force=ipv4

// #define ICMP_ER   0    /* echo reply */
// #define ICMP_DUR  3    /* destination unreachable */
// #define ICMP_SQ   4    /* source quench */
// #define ICMP_RD   5    /* redirect */
// #define ICMP_ECHO 8    /* echo */
// #define ICMP_TE  11    /* time exceeded */
// #define ICMP_PP  12    /* parameter problem */
// #define ICMP_TS  13    /* timestamp */
// #define ICMP_TSR 14    /* timestamp reply */
// #define ICMP_IRQ 15    /* information request */
// #define ICMP_IR  16    /* information reply */
// #define ICMP_AM  17    /* address mask request */
// #define ICMP_AMR 18    /* address mask reply */

// Handling ICMP messages
//
static u8_t icmp_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr) {
    printf("Received ICMP packet\n");

    if (p->len >= sizeof(struct icmp_hdr)) {
        struct icmp_hdr *icmphdr = (struct icmp_hdr *)p->payload;

        print_packet(p);
        printf("ICMP type: %d\n", icmphdr->type);
        
        if (icmphdr->type == ICMP_ECHO) {
            printf("ICMP Echo Request detected from %s\n", ipaddr_ntoa(addr));
        } else {
            printf("ICMP Packet of unidentified type detected from", ipaddr_ntoa(addr));
        }
    } else {
        printf("Received ICMP packet too short from: %s\n", ipaddr_ntoa(addr));
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
    
    // Print a message to indicate that the ICMP listener is up, function ran successfuly
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

int main() {
    

    // We need two buffers, one for the data to send, and one for the data to receive.
    //   uint8_t out_buf [BUF_LEN], in_buf [BUF_LEN];
    //   // Initialize the buffers to 0.
    //   for (u_int8_t i = 0; i < BUF_LEN; ++i) {
    //     out_buf [i] = 0;
    //     in_buf [i] = 0;
    //   }
    //   for (uint8_t i = 0; ; ++i) {
    //     printf ("Sending data %d to SPI Peripheral\n", i);
    //     out_buf [0] = i;
    //     // Write the output buffer to COPI, and at the same time read from CIPO to the input buffer.
    //     spi_write_read_blocking (spi_default, out_buf, in_buf, 1);
    //     // Sleep for some seconds so you get a chance to read the output.
    //     sleep_ms (2 * 1000);
    //   }

    // LWIP network interface
    //
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
    //
    clock_configure_gpin(clk_sys, 20, 50 * MHZ, 50 * MHZ);
    sleep_ms(100);

    // initialize stdio after the clock change
    //
    stdio_init_all();
    sleep_ms(5000);
    
    printf("LAN Interface up, initializing SPI now\n");

    // SPI
    // Have to snprintf string we wish to send through
    //
    stdio_init_all();
    sleep_ms (2 * 1000);
    printf ("Initializing SPI\n");
    // Enable SPI0 at 1 MHz
    spi_init (spi_default, 1 * 1000000);
    // Assign SPI functions to the default SPI pins
    gpio_set_function (PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function (PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function (PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function (PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // initilize LWIP in NO SYS mode
    //
    lwip_init();

    // initialize the PIO base RMII Ethernet network interface
    //
    netif_rmii_ethernet_init(&netif, &netif_config);
    
    // assign callbacks for link and status
    //
    netif_set_link_callback(&netif, netif_link_callback);
    netif_set_status_callback(&netif, netif_status_callback);

    // set the default interface and bring it up
    //
    netif_set_default(&netif);
    netif_set_up(&netif);

    #include "lwip/opt.h"
    #include "lwip/init.h"
    #include "lwip/netif.h"
    #include "lwip/ip4_addr.h"
    #include "lwip/tcpip.h"
    #include "netif/ethernet.h"

    // Definitions here for code clarify, will remove in integration.
    //
    #define MY_IP_ADDR      "192.168.1.100"
    #define MY_NET_MASK     "255.255.255.0"
    #define MY_GATEWAY_ADDR "192.168.1.1"
    ip4_addr_t ipaddr, netmask, gw;

    // If we are using DHCP Server at SR7A, we may use this to lease, 
    // alternatively we set a static IP Address and perform a point to point connection
    //
    //dhcp_start(&netif);
    //httpd_init();

    // Setting static IP Addresses
    //
    IP4_ADDR(&ipaddr, 192,168,1,100);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,1,1);

    // Apply the configuration
    //
    netif_set_addr(&netif, &ipaddr , &netmask, &gw);
    netif_set_up(&netif);

    // Setup ICMP Listener
    //
    setup_icmp_listener();

    // Since LAN8720 performs polling, we can set core1 to perform polling while core0 receives
    //
    multicore_launch_core1(netif_rmii_ethernet_loop);

    // To test for internet connectivity should we connect to router at SR7A, was done for Packet routing but scrapped.
    //
    // ip4_addr_t google_dns;
    // IP4_ADDR(&google_dns, 8,8,8,8); // 8.8.8.8

    while (1) {
        tight_loop_contents();

            // Periodically pings google DNS Server to test for internet connection
            //
            // send_icmp_ping(&netif, &google_dns);
            // sleep_ms(10000); // Periodically ping Google DNS Server to test internet connectivity
    }

    return 0;
}
