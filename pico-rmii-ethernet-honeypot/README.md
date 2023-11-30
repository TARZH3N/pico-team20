# Acknowledgements
Our team would like to thank Sandeep Mistry for which driver files we used.
https://github.com/sandeepmistry/pico-rmii-ethernet

# Functionalities
In our project, we have integrated an ICMP PCB with our LAN8720 interface. This setup is designed to initiate an alarm upon detecting an ICMP packet. The system is calibrated to respond when the number of ICMP packets exceeds 15 (our defined ICMP_RATE) within one second, a threshold we have established based on what we deemed unreasonable for normal traffic. Once this threshold is crossed, this LAN PICO, acting as an I2C Master, will transmit the attacker's IP address to the I2C Slave equipped with the SD Card. This mechanism is pivotal in our approach to manage and mitigate potential network threats.