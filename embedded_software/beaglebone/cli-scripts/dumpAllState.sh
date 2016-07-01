!#/bin/bash

echo "f_dumpCoreState" > /dev/ttyS2
echo "====Core===="
head -5 /dev/ttyS2
echo "f_dumpOrientationState" > /dev/ttyS2
echo "====Orientation===="
head -10 /dev/ttyS2
echo "f_dumpInputState" > /dev/ttyS2
echo "====Input===="
head -7 /dev/ttyS2
echo "f_dumpRawInputState" > /dev/ttyS2
echo "====RawInput===="
head -4 /dev/ttyS2
echo "f_dumpOutputState" > /dev/ttyS2
echo "====Output===="
head -8 /dev/ttyS2

