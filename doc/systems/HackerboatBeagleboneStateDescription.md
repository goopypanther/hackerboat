Hackerboat Beaglebone State Description
=======================================

Start
-----
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Amber

This is the state the Beaglebone enters on power up. It transitions to Self-Test after peripheral initialization.

Self-Test
---------
**System**          | **State**
--------------------|----------
Indicator Lights    | Flashing Amber

In this state, the Beaglebone performs a self-test of its internal systems and peripherals to determine whether it can operate. The test are as follows:

1. Check that it has a valid network
2. Check that it has a connection to the shore station
3. Check that it is receiving communication from the Arduino
4. Check that the GPS has a solid fix

If these tests are successful, the Beaglebone enters the Disarmed state.

Disarmed
--------
**System**          | **State**
--------------------|----------
Indicator Lights    | Flashing Blue

This is the wait state with the physical enable switch in the ‘off’ position. The enable switch state is sensed by the Arduino. 

Halt
----
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Blue

This is the armed wait state. The Beaglebone will transition to either the Waypoint Navigation or Steering states upon shore station command.

Waypoint Navigation
-------------------
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Green

This is the typical autonomous operational mode. The Beaglebone maintains a list of waypoints commanded by the shore station and navigates to each one in order. It provides heading and throttle commands to the Arduino in order to achieve the desired path. Shore commands can switch it to Halt or Steering modes. Loss of GPS will drive it into Fault mode and loss of network connection will send it to Loss of Signal.

Steering
--------
**System**          | **State**
--------------------|----------
Indicator Lights    | Alternating Solid Blue and Green

This mode slaves the steering and throttle to shore commands. Shore commands can switch it to Halt or Waypoint Navigation modes. Loss of GPS will drive it into Fault mode and loss of network connection will send it to Loss of Signal.

Loss of Signal
--------------
**System**          | **State**
--------------------|----------
Indicator Lights    | Flashing Red

This is the state the Beaglebone enters if it loses network. If it was in Waypoint or Steering mode, it navigates to a pre-determined waypoint.

Fault 
-----
**System**          | **State**
--------------------|----------
Indicator Lights    | Solid Red

This is the state the Beaglebone enters if it fails self-test or loses either GPS or contact with the Arduino. 
