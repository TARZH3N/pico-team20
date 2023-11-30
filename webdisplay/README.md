# Commendations

Our team extends sincere commendations to Seth for his exceptional work in implementing a download feature to download files stored in RP2040 directly. His contribution has significantly enhanced the project's capabilities. Regrettably, we were unable to integrate the ICMP Flooding Statistics with his branch within the allocated timeframe. We acknowledge his hardwork and contributions.

# Running
To run this folder, alter your WIFI SSID in main.c, our team opted out of configuring it in our environment variable for code readability.
cmake .
make
pico-team20.uf2 will then be generated.

# Limitations
Our team has identified that text file storing packets captured in AP Monitor mode may not be displayed.

The following fields in lwiopts.h has been set to the maximum threshold
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN 2048
#define TCP_MSS                     3000

Attempts at pagination with cgi.h were also unsuccessful.