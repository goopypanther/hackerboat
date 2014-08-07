Hackerboat Arduino State Description
====================================

These states are divided into three groups.

1. **Initialization** These states are ones entered automatically as a result of powering up the system.
2. **Safe** These states are those where crew action is required to start the prop.
3. **Unsafe** These states are where no on-site crew action is required to start the prop. When the boat enters an unsafe state from a safe state, it must sound the horn for fifteen seconds before entering the state proper.

Power Up
--------
**System**          | **State**
--------------------|----------
Group				| Initialize
Indicator Lights    | Flashing Green
Motor               | Off
Steering            | Off

This state is the state that the Arduino enters upon power-up. It transitions to Self-Test after initializing all peripherals.

Self-Test 
---------
**System**          | **State**
--------------------|----------
Group				| Initialize
Indicator Lights    | Flashing Amber
Motor               | Off
Steering            | On

In this state, the Arduino runs a self-test on its internal systems and determines whether it is suitable for operation. The tests are as follows:

1. Check battery voltage is above 12V
2. Check that the compass is providing a valid, stable output
3. Check that the accelerometer is indicating that the boat is upright and close to level
4. Check that the gyro is indicating that the boat has sufficiently low rates
5. Check that the beaglebone is transmitting
If the tests are successful, the Arduino checks the EEPROM for the last state. If it was 'Armed' or 'Active', it transitions into the appropriate state. Otherwise, it transitions into the 'Disarmed' state.  
If any of the test fail, it transitions into the Fault state.

Disarmed
--------
**System**          | **State**
--------------------|----------
Group				| Safe
Indicator Lights    | Solid Amber
Motor               | Off
Steering            | Off

This is the safed state of the boat. It requires physical activation of the enable switch to transition to unsafe states from here. This is also the state the Arduino transitions to if the e-stop switch is activated. 

Armed
----
**System**          | **State**
--------------------|----------
Group				| Unsafe
Indicator Lights    | Solid Blue
Motor               | Off
Steering            | On

This is the armed waiting state. The steering is active in this state to allow a quick visual check.

Active
--------
**System**          | **State**
--------------------|----------
Group				| Unsafe
Indicator Lights    | Solid Green
Motor               | On
Steering            | On

This is the normal active state. The Arduino is taking course and throttle commands from the beaglebone.

Low Battery
-----------
**System**          | **State**
--------------------|----------
Group				| Unsafe
Indicator Lights    | Alternating Green and Amber
Motor               | Off
Steering            | Off

If the battery voltage falls below 9V, the system enters this state to give the panels time to recharge the batteries.
The Arduino will return to its previous mode when battery voltage recovers to 11V.

Fault
-----
**System**          | **State**
--------------------|----------
Group				| Safe
Indicator Lights    | Flashing Red
Motor               | Off
Steering            | Off

This state is for when the Arduino fails self-test.

Self Recovery
-------------
**System**          | **State**
--------------------|----------
Group				| Unsafe
Indicator Lights    | Flashing Red and White
Motor               | On
Steering            | On

This state is for when the Beaglebone dies or enters Fault state. The boat will attempt self recovery by steering due east at maximum power. 