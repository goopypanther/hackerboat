#!/bin/sh

# set muxes for LEDs
echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad10
echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad0
echo 07 > /sys/kernel/debug/omap_mux/gpmc_a0
echo 07 > /sys/kernel/debug/omap_mux/mcasp0_ahclkx

# set muxes for logic analyzer lines
echo 07 > /sys/kernel/debug/omap_mux/lcd_data0
echo 07 > /sys/kernel/debug/omap_mux/lcd_data1
echo 07 > /sys/kernel/debug/omap_mux/lcd_data2
echo 07 > /sys/kernel/debug/omap_mux/lcd_data3
echo 07 > /sys/kernel/debug/omap_mux/lcd_vsync
echo 07 > /sys/kernel/debug/omap_mux/lcd_pclk
echo 07 > /sys/kernel/debug/omap_mux/lcd_hsync
echo 07 > /sys/kernel/debug/omap_mux/lcd_ac_bias_en

# export gpio pins for LEDs

echo 32 > /sys/class/gpio/export
echo 26 > /sys/class/gpio/export
echo 127 > /sys/class/gpio/export
echo 48 > /sys/class/gpio/export

# export gpio pins for logic analyzer lines

echo 70 > /sys/class/gpio/export
echo 86 > /sys/class/gpio/export
echo 71 > /sys/class/gpio/export
echo 88 > /sys/class/gpio/export
echo 72 > /sys/class/gpio/export
echo 89 > /sys/class/gpio/export
echo 73 > /sys/class/gpio/export
echo 87 > /sys/class/gpio/export

# set output direction for all gpio

echo out > /sys/class/gpio/gpio26/direction
echo out > /sys/class/gpio/gpio32/direction
echo out > /sys/class/gpio/gpio127/direction
echo out > /sys/class/gpio/gpio48/direction
echo out > /sys/class/gpio/gpio70/direction
echo out > /sys/class/gpio/gpio71/direction
echo out > /sys/class/gpio/gpio72/direction
echo out > /sys/class/gpio/gpio73/direction
echo out > /sys/class/gpio/gpio86/direction
echo out > /sys/class/gpio/gpio87/direction
echo out > /sys/class/gpio/gpio88/direction
echo out > /sys/class/gpio/gpio89/direction

# set initial value for all gpio

echo 0 > /sys/class/gpio/gpio26/value
echo 0 > /sys/class/gpio/gpio32/value
echo 0 > /sys/class/gpio/gpio127/value
echo 0 > /sys/class/gpio/gpio48/value
echo 0 > /sys/class/gpio/gpio70/value
echo 0 > /sys/class/gpio/gpio71/value
echo 0 > /sys/class/gpio/gpio72/value
echo 0 > /sys/class/gpio/gpio73/value
echo 0 > /sys/class/gpio/gpio86/value
echo 0 > /sys/class/gpio/gpio87/value
echo 0 > /sys/class/gpio/gpio88/value
echo 0 > /sys/class/gpio/gpio89/value

# start eth0

ifup eth0

# set up UART1

echo 0 > /sys/kernel/debug/omap_mux/uart1_txd
echo 20 > /sys/kernel/debug/omap_mux/uart1_rxd
stty -F /dev/ttyO1 57600

# start socat

socat -d -d -lf socat.log TCP4-LISTEN:5780,fork OPEN:/dev/ttyO1,raw,b57600 &
nice -20 socat -d -d -lf socat.log TCP4-LISTEN:5760,fork EXEC:/home/root/StalkerCopter/stalkerLink/stalkerLink &
