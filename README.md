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

See [./embedded_software](https://github.com/JeremyRuhland/hackerboat/tree/master/embedded_software)

Doxygen documentation of beaglebone software may be generated with `make doxy` in the `./embedded_software/beaglebone` folder and found at `./embedded_software/beaglebone/doc`.

Host Software
-------------
To be added soon (probably Q Ground Control stuff).

See [qGroundControl](http://www.qgroundcontrol.org)

Hardware Schematics
-------------------
On-ship hardware for connecting beaglebone, arduino and power systems together.

See [./hardware](https://github.com/JeremyRuhland/hackerboat/tree/master/hardware)

Boat System Documentation
-------------------------
View under [./doc](https://github.com/JeremyRuhland/hackerboat/tree/master/doc)
