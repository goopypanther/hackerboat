Hackerboat Control Software
===========================

The hackerbot labs autonomous hackerboat is a modified kayak with 900MHz/2.4GHz radios, electric motor, IMU, scientific sensors & solar power rechargeable batteries.

The control system uses a beaglebone & arduino mega which communicate using the MAVLink protocol with a land based host computer.

Embedded Software
-----------------

 * Beaglebone
  * MAVLink-over-IP communication with host PC
  * IMU/GPS tracking & waypoint navigation
  * MAVLink-over-UART communication with arduino
 * Arduino
  * Motor control
  * Magnetic navigation
  * Ship health & sensor feedback

Host Software
-------------

To be added soon (probably Q Ground Control stuff).

Hardware Schematics
-------------------

On-ship hardware for connecting beaglebone, arduino and power systems together.
