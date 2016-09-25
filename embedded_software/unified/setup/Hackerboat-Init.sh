#!/bin/sh

# Load necessary cape(s)

sudo sh -c "echo BB-ADC > /sys/devices/platform/bone_capemgr/slots"

# UART setup

# RS-485
config-pin P9.24 uart
config-pin P9.26 uart

# RC/Cell
config-pin P8.37 uart
config-pin P8.38 uart

# GNSS
config-pin P9.11 uart
config-pin P9.13 uart
stty -F /dev/ttyS4 9600 -echo

# I2C setup
# I2C-2 pins are fixed

config-pin P9.17 i2c
config-pin P9.18 i2c

# PWM setup 

config-pin P9.16 pwm

# start gpsd

gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0
