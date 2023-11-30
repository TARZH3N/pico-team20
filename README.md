# pico-team20
INF2004 IS PROJECT

## First Pico - WiFi Sniffing + Web Server 
1. Enables AP and Monitor mode of WiFi module to capture packets from connected clients.
2. Stores captured packets into SD card.
3. Receives alert of ICMP flood detection from 2nd pico via i2C (Pico 1 is the i2C Slave). 
4. Displays content of captured packets and any alerts of ICMP flood detection on web server.

## Second Pico - Ethernet Honeypot
1. Enables packet capture from client connected via Ethernet interface.
2. Analyzes ICMP packets to detect ICMP flood.
3. Sends ICMP flood detection alert to 1st pico via i2C (Pico 2 is the i2C Master).

## Diagrams
### Block Diagram for both Picos
<img src="/Block%20Diagram.png"/>

### Flow Chart for Pico 1
<img src="/FlowChartPico1.png"/>

### Flow Chart for Pico 2
<img src="/FlowChartPico2.png"/>
