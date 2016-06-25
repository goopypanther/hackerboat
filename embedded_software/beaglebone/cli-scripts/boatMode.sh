#!/bin/bash
echo "f_writeBoatMode?params=$1" > /dev/ttyS2
echo ""
echo "==============================="
head -2 /dev/ttyS2

