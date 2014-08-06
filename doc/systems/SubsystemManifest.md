Subsystem Manifest
==================

This is a list of all subsystems on the boat with a thumbnail description of each and their major components.

1 Control System
================
The control system comprises those components involved in navigating the vessel and maintaining its state.

1.1 Control Box
---------------
The control box contains a large interconnect PCB (described in [§ 1.1.5](#115-interconnect)) with an Arduino Mega, a Beaglebone, a 9-DoF IMU, and a GPS shield attached to it. It implements all guidance, navigation, and control functions.

1.1.1 Beaglebone
----------------
The Beaglebone runs software implementing the higher level navigation and vehicle management functions. The GPS is connected to the Beaglebone, as are the network interface and the Arduino. All other I/O is handled by the Arduino directly or through the interconnect PCB. I/O is as follows:

 * Ethernet link to radio
 * Serial link to the GPS
 * Serial link to the Arduino Mega
See [HackerboatBeagleboneStateDescription.md](./HackerboatBeagleboneStateDescription) and hackerboat/doc/systems/beaglebone-states.graphml for more information.

1.1.2 Arduino
-------------
The Arduino Mega handles all low level I/O and basic vehicle navigation functions. It attempts to steer the course provided by the Beaglebone or shore command by manipulating the steering servo. It sets the throttle command transmitted to it by the Beaglebone or shore station. I/O is as follows:

 * Serial link to Beaglebone
 * I2C link to the IMU
 * 8 relay channels with status feedback
 * 8 auxiliary channels with status and current feedback
 * Digital link to Arduino status lights
 * Digital link to Beaglebone status lights
 * PCM link to steering servo
 * Analog battery voltage monitor (internal)
 * 4 ea external current/voltage monitors

See [HackerboatArduinoStateDescription.md](./HackerboatArduinoStateDescription) and hackerboat/doc/systems/arduino.states.graphml for more information.

1.1.3 GPS
---------
This unit is an Adafruit Ultimate GPS Logger Shield (https://learn.adafruit.com/adafruit-ultimate-gps-logger-shield). Only the GPS connection is hooked up, and then strictly through the serial connection. The PPS and SD Card are not connected.
The GPS is mounted to the carrier board below the Interconnect and is connected to the Beaglebone via the Interconnect with wires soldered in place.
The GPS relies on an external antenna mounted near the bow of the boat.

1.1.4 IMU
---------
This unit is an Adafruit 9-DOF IMU Breakout (https://learn.adafruit.com/adafruit-9-dof-imu-breakout) mounted to the Interconnect via a pin header and mechanical connection. It provides compass, orientation, and rate data.

1.1.5 Interconnect
------------------
The Interconnect PCB provides connections between the Beaglebone, the Arduino, and the various sensors and switches they rely on. It also provides regulated power for the Beaglebone and the steering servo derived from the 12V unregulated battery voltage. Its functions are as follows:

 * Electrical interconnects, as noted above.
 * 5V power supply for the Beaglebone
 * 7.4V power supply for the steering servo
 * Eight high-side, high current drivers to drive relays from the main battery voltage.
 * Fault feedback from the relay drivers
 * Eight high-side, high current drivers to drive auxiliary loads
 * Fault feedback from the auxiliary drivers
 * Current feedback from the auxiliary drivers
 * Ready access to all Arduino and Beaglebone pins
 * Status LEDs for all power rails and relay ports

1.1.6 Mechanical Arrangements
-----------------------------
All of the above parts are mounted within a Bud Industries PN-1329-C waterproof polycarbonate box with a clear lid.

 * The Interconnect is mounted to a laser-cut wood carrier plate.
 * The Beaglebone and the Arduino are attached to the underside of the Interconnect.
 * The IMU is connected to the top side.
 * The GPS is connected to the wood carrier plate
 * All connections are drilled through the sides of the solid lower half of the box.

See hackerboat/doc/systems/InterfaceControlDocument.xlsx for more information on the physical connections.

1.2 E-stop/Enable Buttons
-------------------------
These put the vessel in disarmed state or let it out of disarmed state. One commands the transition to Disarmed and one commands the transition to Halt. Each is a SPDT On-(On) pushbutton. The normal leg is connected to GND and the momentary leg is connected to +5V. The buttons are covered with a wave guard that drains.

1.3 Beaglebone Status Lights
----------------------------
Strip of 4 waterproofed addressable RGB LEDs, mounted externally and aft.

1.4 Arduino Status Lights
-------------------------
Strip of 4 waterproofed addressable RGB LEDs, mounted externally and aft.

1.5 Current/Voltage Monitors
----------------------------
Boatse 0.2: Only the motor monitor is installed. This monitors the voltage and current on the motor, and is mounted between the relay box and the main power bus. It is a 30A/20V inline unit from Logos Electromechanical.
Boatse 0.3: Additional monitors are mounted in the main battery and charge circuits.

2 *Shore Station*
=================

2.1 *Laptop*
------------

2.2 *QGroundControl*
--------------------

2.3 *900 MHz Radio*
-------------------

3 Propulsion
============

3.1 Motor
---------
Propulsion is provided by a Minn Kota Endura C2-30 12V electric trolling motor.
Speed and direction control on this motor is provided by a nine-position rotary switch. It works by alternately connecting the four wires (red, yellow, black, and white) to power, ground, and each other. See hackerboat/doc/motor/Minn Kota Endura C2-30 wiring for wiring details.
The motor is rated up to 30A, but in service draw is currently unknown.
The motor has a custom-built tiller mounted to the top of the shaft.

3.2 Relay Box
-------------
The nine-position switch can be replicated in every detail with seven SPDT relays. Two of the relays are ganged together to provide a DPDT function for reversing. Of the remaining five, one controls current flow to each of the red, white, and yellow wires. The last two provide the capability to short the red wire to the white wire or the yellow wire to the red wire.
Relay drive is provided by the control box via interconnect wires.
See hackerboat/doc/systems/InterfaceControlDocument.xlsx for more information on the external connections.

3.3 Servo
---------
The steering servo is a Hitec HS-5646 servo (http://www.servocity.com/html/hs-5646wp_servo.html), which is IP67 rated. It drives the tiller through a gear reduction system (http://www.servocity.com/html/spg400a-bm-cr_continuous_rotat.html) which can be adjusted to give a number of different gear ratios from 2.8:1 to 7:1.
A 3” aluminum servo arm bolts to the output gear of the servo to provide a steering arm.
The current tiller connection must be disconnected by unscrewing a nyloc nut in order to stow the motor. This is suboptimal, but will not be rectified prior to Boatse 0.3.

3.4 Mechanical Mounting
-----------------------
The motor is provided with an outboard-style screw clamp arrangement that connects it to the motor mount bulkhead. This mount allows it to swivel freely for steering, or be raised and lowered for convenience in handling during launch and recovery.
The servo is mounted to a specially designed plate epoxied to the top of the motor bulkhead. The servo arm is connected to the tiller via an adjustable length mounting link. This link uses ball ends to absorb the misalignment between the two.

4 Communications
================

4.1 Radio Box
-------------

4.1.1 900 MHz
-------------
The 900 MHz radio provides ship to shore communications.

4.1.2 2.4 GHz
-------------
The 2.4 GHz radio serves as the primary Ethernet switch and allows the use of standard WiFi accessories such as cameras.

4.1.3 *Boost Regulator*
-----------------------

4.2 Antenna Mast
----------------
The antenna mast is a xxx” diameter aluminum tube that elevates the 900 MHz antenna well above the boat in order to

5 Power
=======

5.1 Main Bus
------------

5.1.1 Boatse 0.2
----------------
The main power distribution bus consists of a sixteen circuit 0.375” pitch terminal strip mounted to the forward side of the center bulkhead. The bus voltage is nominal 12V, with a valid range of 8V to 14V.

5.1.2 Boatse 0.3
----------------
This will be relocated somewhere easier to reach.

5.2 Batteries
-------------
The boat is powered by a pair of 12V truck batteries. ***Insert part numbers etc here*** These batteries are wired in parallel to provide a single nominal 12V bus.

5.3 Charger
-----------

5.4 Solar Panels
----------------
Boatse 0.2: Solar panels are not mounted
Boatse 0.3: Solar panels are mounted as described below

5.4.1 Panel 1
-------------

5.4.2 Panel 2
-------------

5.4.3 Panel 3
-------------

5.4.4 Mounting Arrangements
---------------------------

6 Structures
============

6.1 Initial Hull
----------------

6.2 Motor Mount
---------------

6.3 Bulkheads
-------------

6.4 Keelson
-----------

6.5 Keel
--------

7 Miscellaneous
===============

7.1 Bilge Pump
--------------
The bilge pump is a 12V marine unit controlled by a float switch. The pump is mounted on the aft side of the center bulkhead directly to starboard of the keelson. The switch is mounted on the port side and wired in series with the pump. The pump is wired directly into the main power distribution bus. 
