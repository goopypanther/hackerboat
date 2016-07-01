#!/bin/bash
echo "f_writeRudder?params=$1" > /dev/ttyS2
echo ""
echo "==============================="
head -2 /dev/ttyS2

