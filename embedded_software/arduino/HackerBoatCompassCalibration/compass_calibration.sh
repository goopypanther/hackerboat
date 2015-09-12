#!/bin/sh

FILEPATH="compass_calibration.txt"

echo "" >> $FILEPATH
echo "--------------------------------------------------------------------------------" >> $FILEPATH
echo "" >> $FILEPATH

cat /dev/ttyO1 >> $FILEPATH

