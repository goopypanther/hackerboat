#!/bin/sh

# set up wifi
#ifup wlan0=wlan_boat

# enable UART4 (GPS)

config-pin P9.11 uart
config-pin P9.13 uart
stty -F /dev/ttyS4 9600 -echo

# enable UART1 (Log)

config-pin P9.24 uart
config-pin P9.26 uart
stty -F /dev/ttyS1 115200 -echo

# enable UART2 (Arduino REST)

config-pin P9.21 uart
config-pin P9.22 uart
#stty -F /dev/ttyS2 115200 -echo

# setup arduino reset pin and hold arduino in reset

config-pin P9.15 low

# start lighttpd

# start daemons

# take arduion out of reset

config-pin P9.15 high
