Hackerboat Arduino State Description
====================================

Start
-----
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Amber
Motor               | Off
Steering            | Off

This state is the state that the Arduino enters upon power-up. It transitions to Self-Test after initializing all peripherals.

Self-Test 
---------
**System**          | **State**
--------------------|----------
Indicator Lights    | Flashing Amber
Motor               | Off
Steering            | On

In this state, the Arduino runs a self-test on its internal systems and determines whether it is suitable for operation. The tests are as follows:

1. Check battery voltage is above 12V
2. Check that the compass is providing a valid, stable output
3. Check that the accelerometer is indicating that the boat is upright and close to level
4. Check that the gyro is indicating that the boat has sufficiently low rates
5. Check that the beaglebone is transmitting
If the test is successful, the Arduino transitions into the Disarmed state. Otherwise, it transitions into the Fault state.

Disarmed
--------
**System**          | **State**
--------------------|----------
Indicator Lights    | Flashing Amber
Motor               | Off
Steering            | On

Halt
----
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Blue
Motor               | Off
Steering            | On

This is the inactive waiting state.

Steering
--------

Low Battery
-----------

Fault
-----

Panic
-----
