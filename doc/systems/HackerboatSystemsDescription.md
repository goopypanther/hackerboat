Structures
==========
The structure is built around a surplus two-hatch fiberglass kayak obtained from Craigslist. 

Dimensions
----------

Initial Condition
-----------------

Structural Additions
--------------------

Panel Mounting
--------------

Keel
----

Hatches
-------
Deck over each cockpit and install this:
http://www.jmsonline.net/boat-hardware/ventilation/hatches-and-accessories/bomar-inspection-hatch-4fw1326.htm

Accessory Mounting Arrangements
-------------------------------

Propulsion
==========
Propulsion is provided by a Minn Kota Endura C2-30 12V electric trolling motor. See https://docs.google.com/spreadsheets/d/1InIYZVhCP4nxBoPUhnIeGG7QbtK-sMKTjdIkPkHh7EY for wiring information and http://tufox.com/hobie/TrollingPerformance.html for performance data on a similar motor.
Speed control is provided by a nine-position switch arrangement. This can be duplicated with a set of relays (schematic forthcoming) or through stepper/servo drive connected to the existing switch. I like the relays better; acceptable sized automotive-style relays are typically cheap on eBay. 

Steering
========
The Minn Kota is designed to be steered by rotating the shaft. The steering drag is adjustable, but defaults to very low. There not much plane area to it, so I suspect that wave loads will be correspondingly low.
Steering shall be through a cable arrangement. Cables will be attached from the quadrant driven by the steering motor to the quadrant or arm attached to the top of the motor steering shaft. Each side of the cable will have a turnbuckle to adjust tension. Both turnbuckles will be safety wired once set to prevent movement. 
The top of the motor shaft has a bolt hole designed for bolting the head onto the shaft. This hole will be used to secure the aft steering quadrant.
Steering motion will be provided by a standard-size servo that’s IP67 rated (http://www.servocity.com/html/hs-5646wp_servo.html). This will be mated to a gear reduction unit designed to mount onto a flat plate (http://www.servocity.com/html/spg400a-bm-cr_continuous_rotat.html). Actobotics sells a 6” aluminum arm cut from beefy plate to round out the mechanism.

Controls
========
The control system will consist of two elements -- a real time element that does low-level steering and power control and a high-level element that handles higher level navigation, ship to shore communications, and similar operations. These two elements will be mounted to a single motherboard that aggregates all computing functions on board. 

Real Time Element
-----------------
This part will handle the following tasks:

 * Steering a given magnetic heading
 * Selecting the power setting that provides greatest range for the current charge conditions.
 * Monitoring motor and steering gear health
 * Monitoring battery & solar panel status
 * Monitoring IMU & compass output

Intended hardware for this element is an Arduino Uno or equivalent board. It connects to the high level element via a 3.3V TTL serial link over the motherboard. All necessary interface electronics will be part of the motherboard.

High Level Element
------------------
This part will handle the following tasks:

 * Waypoint navigation
 * Coordinating ship to shore communications
 * Data logging
 * GPS interface

Motherboard
-----------
The motherboard provides the following functions:

 * Mechanical support of real time and high level elements.
 * Interconnect between real time and high level elements. 
 * IMU interface -- Adafruit 10-DoF IMU (https://www.adafruit.com/products/1604)
 * GPS interface -- Adafruit GPS Breakout (https://www.adafruit.com/products/746)
 * Steering servo interface
 * Controls & accessory power supply

Navigation
==========

Power
-----
The power for the boat comes from a pair of 120W (rated) solar panels mounted on the top of the hull. (Kindly loaned, along with the batteries and charger, by Myles Conley)
These panels charge a pair of 12V (??)Ah deep-cycle lead-acid batteries mounted in the forward compartment, via a (**Myles, please fill this in**) charger. The two batteries are wired in parallel to provide a single 12V bank of (??)Ah.

Motor Power
-----------
Power for the motor is drawn directly from the batteries via a 50A resettable fuse. (Well, if I can find one --- the biggest I have found available so far is a 40A, which is probably good enough for our intended draw. Digikey lists a 50A in its catalog, but it is not in stock.) The motor wiring from the battery is 12ga  and goes from the battery to the fuse and then through the center bulkhead to the motor speed control.

Controls Power
--------------

Accessory Power
---------------

Communications
==============

Onboard
-------

Main Network
------------

Emergency Backup/Locator
------------------------
