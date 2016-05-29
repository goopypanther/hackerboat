#!/bin/sh

#disable UART1 (Arduino Log)

config-pin P9.24 in
config-pin P9.26 in

# disable UART2 (Arduino REST)

config-pin P9.21 in
config-pin P9.22 in

