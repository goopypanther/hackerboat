#!/bin/sh
# set up UART

echo 0 > /sys/kernel/debug/omap_mux/uart1_txd
echo 20 > /sys/kernel/debug/omap_mux/uart1_rxd
stty -F /dev/ttyO1 57600

echo 0 > /sys/kernel/debug/omap_mux/uart2_txd
echo 20 > /sys/kernel/debug/omap_mux/uart2_rxd
stty -F /dev/ttyO2 57600

echo 0 > /sys/kernel/debug/omap_mux/uart4_txd
echo 20 > /sys/kernel/debug/omap_mux/uart4_rxd
stty -F /dev/ttyO4 57600

# start mavlink
./mavlink_udp /dev/ttyO4 /dev/ttyO1 /dev/ttyO2
