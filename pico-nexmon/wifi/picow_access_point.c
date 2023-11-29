/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


// Import required libraries
//
#include <string.h>
#include <hardware/timer.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "dhcpserver.h"
#include "dnsserver.h"
#include <stdio.h>
#include "sd_card.h"
#include "ff.h"

#include "i2c_slave.h"
#include "i2c_fifo.h"

// Define constants
//
#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define LED_TEST_BODY "<html><body><h1>Hello from Pico W.</h1><p>Led is %s</p><p><a href=\"?led=%d\">Turn led %s</a></body></html>"
#define LED_PARAM "led=%d"
#define LED_TEST "/ledtest"
#define LED_GPIO 0
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" LED_TEST "\n\n"

// Enumeration for monitor mode types
//
#define MONITOR_DISABLED        0
#define MONITOR_IEEE80211       1
/* RADIOTAP MODE REQUIRES A NEXMON FW! */
#define MONITOR_RADIOTAP        2
#define MONITOR_LOG_ONLY        16

#define WRITE_QUEUE_SIZE        50

uint queueHead = 0;
uint queueTail = 0;

//I2C Slave Variables
//
static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz
static const uint I2C_SLAVE_SDA_PIN = 4;
static const uint I2C_SLAVE_SCL_PIN = 5;

//Writing Flags for write to sd card
//
uint writing = 0;
uint added = 0;

// List of Wi-Fi channels to be used for monitoring mode
//
uint8_t chan_idx = 0;
uint32_t channels[] = {1, 6, 11};

// Struct for write queue
//
typedef struct {
    char filename[32];  
    uint8_t *data;
    size_t data_len;
} WriteQueueItem;

// Initialisation of writeQueue array with set write queue size
//
WriteQueueItem writeQueue[WRITE_QUEUE_SIZE];

// Timer struct for beaconTime to beacon
//
struct repeating_timer beaconTime;

// Struct for I2C Slave
//
static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;

// Arrays for frame type and subtype names
//
const char *frame_type_names[3] = {
            "Management",
            "Control",
            "Data"
        };

// Types of WiFi Frames defined in IEEE 802.11 Standard
//
const char *frame_subtype_names[4][16] = {
        {
            "Association Request", "Association Response", "Reassociation Request", "Reassociation Response",
            "Probe Request", "Probe Response", "Timing Advertisement", "Reserved",
            "Beacon", "ATIM", "Disassociation", "Authentication", "Deauthentication", "Action", "Action No Ack (NACK)", "Reserved"
        },
        {
            "Reserved", "Reserved", "Trigger[3]", "TACK",
            "Beamforming Report Poll", "VHT/HE NDP Announcement", "Control Frame Extension", "Control Wrapper",
            "Block Ack Request (BAR)", "Block Ack (BA)", "PS-Poll", "RTS", "CTS", "ACK", "CF-End", "CF-End + CF-ACK"
        },
        {
            "Data", "Reserved", "Reserved", "Reserved",
            "Null (no data)", "Reserved", "QoS Data", "QoS Data + CF-ACK",
            "QoS Data + CF-Poll", "QoS Data + CF-ACK + CF-Poll", "QoS Null (no data)", "Reserved", "QoS CF-Poll (no data)", "QoS CF-ACK + CF-Poll (no data)", "Reserved", "Reserved"
        },
        {
            "DMG Beacon", "S1G Beacon", "Reserved", "Reserved",
            "Reserved", "Reserved", "Reserved", "Reserved",
            "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"
        }
    };

// Function to enqueue writing processes
//
bool enqueue_write(const char* filename, const uint8_t* data, size_t len) {
    if ((queueTail + 1) % WRITE_QUEUE_SIZE == queueHead) {
        // Queue is full
        //
        return false;
    }

    WriteQueueItem *item = &writeQueue[queueTail];
    strncpy(item->filename, filename, sizeof(item->filename));

    // Allocate memory for data
    //
    item->data = malloc(len);  
    if (item->data == NULL) {
        // Memory allocation failed
        //
        return false;
    }

    memcpy(item->data, data, len);
    item->data_len = len;

    queueTail = (queueTail + 1) % WRITE_QUEUE_SIZE;
    return true;
}

// Function to write data to a file
//
void write_to_file(const char *filename, const uint8_t *data, size_t len) {

    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);

    // Check if the file could be opened successfully
    //
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        return;
    }

    // Iterate through the binary data and write it to the file as hexadecimal values
    //
    for (size_t i = 0; i < len; ++i) {
        if (f_printf(&fil, "%02x ", data[i]) < 0) {
            printf("ERROR: Could not write to file\r\n");
            break;
        }
    }

    // Add a newline after writing the data
    //
    f_printf(&fil, "\n"); 

    // Close the file
    //
    fr = f_close(&fil);

    // Check if the file could be closed successfully
    //
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
    }

}

// Structure for TCP server state
//
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
    async_context_t *context;
} TCP_SERVER_T;

// Structure for TCP connection state
//
typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

// Function to handle closing a client connection
//
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    
    // Check if the client PCB is valid
    //
    if (client_pcb) {

        // Ensure that the connection state matches the client PCB
        //
        assert(con_state && con_state->pcb == client_pcb);

        // Remove callback functions and references to the client PCB
        //
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);

        // Attempt to close the client PCB
        //
        err_t err = tcp_close(client_pcb);

        // Check if the close operation failed
        //
        if (err != ERR_OK) {

            // Print an error message and abort the connection if closing fails
            //
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }

        // Free the memory allocated for the connection state
        //
        if (con_state) {
            free(con_state);
        }
    }

    // Return the final error status after attempting to close the connection
    //
    return close_err;
}

// Function to close the TCP server
//
static void tcp_server_close(TCP_SERVER_T *state) {

    // Check if the server PCB is valid
    //
    if (state->server_pcb) {

        // Remove callback functions and references to the server PCB
        //
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);

        // Set the server PCB to NULL after closing
        //
        state->server_pcb = NULL;
        
    }
}

// Callback function when data is sent from the server to the client
//
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {

    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    // Print debug information about the sent data length
    //
    DEBUG_printf("tcp_server_sent %u\n", len);

    // Track the amount of data sent
    //
    con_state->sent_len += len;

    // Check if all data has been sent
    //
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }

    // Return ERR_OK to signal successful handling of the sent data
    //
    return ERR_OK;
}

// Function to test server content based on HTTP request
//
static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len) {
   
    int len = 0;

    // Check if the request is for LED testing
    //
    if (strncmp(request, LED_TEST, sizeof(LED_TEST) - 1) == 0) {

        // Get the state of the LED
        //
        bool value;
        cyw43_gpio_get(&cyw43_state, LED_GPIO, &value);
        int led_state = value;

        // Check if the user provided parameters
        //
        if (params) {
            int led_param = sscanf(params, LED_PARAM, &led_state);
            if (led_param == 1) {

                // Update LED state based on user input
                //
                if (led_state) {

                    // Turn LED on
                    //
                    cyw43_gpio_set(&cyw43_state, 0, true);
                } else {

                    // Turn LED off
                    //
                    cyw43_gpio_set(&cyw43_state, 0, false);
                }
            }
        }

        // Generate result based on LED state
        //
        if (led_state) {
            len = snprintf(result, max_result_len, LED_TEST_BODY, "ON", 0, "OFF");
        } else {
            len = snprintf(result, max_result_len, LED_TEST_BODY, "OFF", 1, "ON");
        }
    }

    // Return the length of the generated result
    //
    return len;
}

// Callback function when data is received by the server
//
err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {

    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    // Check if the received data buffer is empty, indicating the client has closed the connection
    //
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }

    // Ensure that the connection state matches the client PCB
    //
    assert(con_state && con_state->pcb == pcb);

    if (p->tot_len > 0) {

        // Print debug information about the received data
        //
        DEBUG_printf("tcp_server_recv %d err %d\n", p->tot_len, err);

#if 0
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DEBUG_printf("in: %.*s\n", q->len, q->payload);
        }
#endif
        // Copy the request into the buffer
        //
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle the HTTP GET request
        //
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + space
            char *params = strchr(request, '?');

            // Parse parameters from the request
            //
            if (params) {
                if (*params) {
                    char *space = strchr(request, ' ');
                    *params++ = 0;
                    if (space) {
                        *space = 0;
                    }
                } else {
                    params = NULL;
                }
            }

            // Generate content based on the request and parameters
            //
            con_state->result_len = test_server_content(request, params, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s?%s\n", request, params);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            // Check that there is enough buffer space for the result data
            //
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            // Generate web page
            //
            if (con_state->result_len > 0) {

                // Generate HTTP response headers
                //
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                    200, con_state->result_len);

                // Check that there is enough buffer space for the header data
                //
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            } else {

                // Send a redirect response
                //
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                    ipaddr_ntoa(con_state->gw));
                DEBUG_printf("Sending redirect %s", con_state->headers);
            }

            // Send the HTTP response headers to the client
            //
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Send the HTTP response body to the client
            //
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }

        // Notify the TCP stack that data has been received
        //
        tcp_recved(pcb, p->tot_len);
    }

    // Free the received data buffer
    //
    pbuf_free(p);

    // Return ERR_OK to indicate successful handling of the received data
    //
    return ERR_OK;
}

// Callback function when a poll event occurs on the server
//
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {

    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    // Print debug information about the polling event
    //
    DEBUG_printf("tcp_server_poll_fn\n");

    // Close the client connection by disconnecting
    //
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

// Callback function when an error occurs on the server
//
static void tcp_server_err(void *arg, err_t err) {

    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    // Check if the error is not an abort error
    //
    if (err != ERR_ABRT) {

        // Print debug information about the error
        //
        DEBUG_printf("tcp_client_err_fn %d\n", err);

        // Close the client connection due to the error
        //
        tcp_close_client_connection(con_state, con_state->pcb, err);

    }

}

// Callback function when a client connection is accepted by the server
//
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {

    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;

    // Check for errors in accepting the client connection
    //
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }

    // Print debug information about the new client connection
    //
    DEBUG_printf("client connected\n");

    // Create the state for the connection
    //
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }

    // Initialize connection state
    //
    con_state->pcb = client_pcb;
    con_state->gw = &state->gw;

    // Setup connection to client
    //
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    // Return ERR_OK to indicate successful handling of the new connection
    //
    return ERR_OK;

}

// Function to initialize and open the TCP server
//
static bool tcp_server_open(void *arg, const char *ap_name) {

    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;

    // Print debug information about starting the server on a specific port
    //
    DEBUG_printf("starting server on port %d\n", TCP_PORT);

    // Create a new TCP PCB (Protocol Control Block) with IP type set to IPADDR_TYPE_ANY
    //
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);

    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    // Bind the PCB to the specified IP address and port
    //
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);

    if (err) {
        DEBUG_printf("failed to bind to port %d\n",TCP_PORT);
        return false;
    }

    // Set up the server PCB to listen for incoming connections with a backlog of 1
    //
    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    // Set the argument and callback function for the server PCB
    //
    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    // Print information for the user to connect to the server
    //
    printf("Try connecting to '%s' (press 'd' to disable access point)\n", ap_name);

    // Return true to indicate successful server initialization
    //
    return true;
}

// Worker function for handling key press events
//
void key_pressed_worker_func(async_context_t *context, async_when_pending_worker_t *worker) {

    // Ensure the worker has valid user data
    //
    assert(worker->user_data);

    // Print a message indicating that Wi-Fi is being disabled
    //
    printf("Disabling wifi\n");

    // Disable access point mode in the Wi-Fi module
    //
    cyw43_arch_disable_ap_mode();

    // Set the 'complete' flag in the TCP server state to true
    //
    ((TCP_SERVER_T*)(worker->user_data))->complete = true;

}

// Definition of the key_pressed_worker structure
//
static async_when_pending_worker_t key_pressed_worker = {
        .do_work = key_pressed_worker_func
};

// Function to handle key press events
//
void key_pressed_func(void *param) {

    // Ensure the parameter is not NULL
    //
    assert(param);

    // Get any pending key press without waiting
    //
    int key = getchar_timeout_us(0); 

    // Check if the key pressed is 'd' or 'D'
    //
    if (key == 'd' || key == 'D') {

        // Create a worker event to disable access point mode in the Wi-Fi module
        //
        async_context_set_work_pending(((TCP_SERVER_T*)param)->context, &key_pressed_worker);
    
    }
}

// Callback function for handling monitor mode events
//
void monitor_mode_cb(void *data, int itf, size_t len, const uint8_t *buf) {

    // Extract information from the received frame
    //
    uint16_t offset_80211 = 0;

    // Adjust the offset if the monitor mode is set to RADIOTAP
    //
    if (cyw43_state.is_monitor_mode == MONITOR_RADIOTAP)
        offset_80211 = *(uint16_t*)(buf+2);

    // Extract frame type and subtype from the frame header
    uint8_t frame_type = buf[offset_80211] >> 2 & 3;
    uint8_t frame_subtype = buf[offset_80211] >> 4;

    // Print information about the received frame
    //
    printf("Frame type=%d (%s) subtype=%d (%s) len=%d data=", 
           frame_type, frame_type_names[frame_type], frame_subtype, 
           frame_subtype_names[frame_type][frame_subtype], len);

    
    // Print the data bytes of the frame
    //
    for (size_t i = 0; i < len; ++i) {
        printf("%02x ", buf[i]);
    }

    printf("\n");

    // Determine the appropriate file based on frame subtype
    //
    char filename[20];
    snprintf(filename, sizeof(filename), "packet_%d.txt", frame_subtype);

    // Call function to queue data for writing to file
    //
    enqueue_write(filename, buf, len);
}

// Function for handling i2c transmission from Master
//
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    // Master has sent some data
    //
    case I2C_SLAVE_RECEIVE: 
        if (!context.mem_address_written) {
            context.mem_address = i2c_read_byte(i2c);
            context.mem_address_written = true;
        } else {
            context.mem[context.mem_address] = i2c_read_byte(i2c);
            context.mem_address++;
        }
        break;
    // Master Requesting Data
    //
    case I2C_SLAVE_REQUEST:
        // No request needed
        //
        break;
    // Master has signalled Stop / Restart
    // 
    case I2C_SLAVE_FINISH: 
        // Queue data for writing
        //
        enqueue_write("i2c_data.txt", context.mem, context.mem_address);
        
        //Signal that data has been added to queue
        //
        if (!added) {
            added = 1;
        }

        context.mem_address_written = false;
        context.mem_address = 0;
        break;
    default:
        break;
    }
}

// Function to write data that is in queue
//
void processWriteQueue() {
    // Check if there is a concurrent write operation
    //
    if (!writing) {
        writing = 1;
        while (queueHead != queueTail) {
            WriteQueueItem *item = &writeQueue[queueHead];
            write_to_file(item->filename, item->data, item->data_len);
            free(item->data);  // Free the allocated memory

            queueHead = (queueHead + 1) % WRITE_QUEUE_SIZE;
        }

        // Reset Flags
        //
        writing = 0;
        added = 0;
    }
}

// Function to set up I2C Slave
//
static void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}

// Function called by beaconTime repeating alarm
//
bool beaconTimer() {
    cyw43_wifi_ap_set_channel(&cyw43_state, channels[chan_idx]);
    chan_idx = (chan_idx + chan_idx) % (sizeof(channels)/sizeof(channels[0]));
}

// Main function
//
int main() {

    // Initialization of standard I/O and file system variables
    //
    stdio_init_all();

    // Initialization of I2C Slave
    //
    setup_slave();

    // Initialization of file system variables and structures
    //
    FRESULT fr;
    DIR dir;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    static FILINFO fno;

    // Definition of the TCP server state
    //
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }
    
    // Initialization of CYW43 Wi-Fi chip
    //
    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return 1;
    }

    // Setting up key press event handling
    //
    state->context = cyw43_arch_async_context();
    key_pressed_worker.user_data = state;
    async_context_add_when_pending_worker(cyw43_arch_async_context(), &key_pressed_worker);
    stdio_set_chars_available_callback(key_pressed_func, state);

    // Configuration for the Wi-Fi access point
    //
    const char *ap_name = "pico test";
#if 1
    const char *password = "12345678";
#else
    const char *password = NULL;
#endif

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    // Setting up IP address and netmask for the access point
    //
    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the DHCP server for dynamic IP assignment
    //
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the DNS server for domain name resolution
    //
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    // Attempt to open the TCP server
    //
    if (!tcp_server_open(state, ap_name)) {
        DEBUG_printf("failed to open server\n");
        return 1;
    }

    

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

    state->complete = false;

    // Main loop
    //
    cyw43_set_monitor_mode(&cyw43_state, MONITOR_IEEE80211, monitor_mode_cb);

    // Repeating timer to beacon
    //
    add_repeating_timer_ms(-200, beaconTimer, NULL, &beaconTime);

    // Repeatedly call for writing to sd card from queue
    //
    while(true){
        // Check if data has been added to queue
        //
        if (added) {
            processWriteQueue();
            sleep_ms(10);
        }
    }

    // Cleanup and deinitialization before exiting the program
    //
    tcp_server_close(state);
    dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    return 0;
}

/*** end of file ***/