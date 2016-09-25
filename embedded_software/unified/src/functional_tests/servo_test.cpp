/******************************************************************************
 * Hackerboat Beaglebone Relay Functional Test program
 * servo_test.cpp
 * This program is a functional test of the servo subsystem
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include "hal/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include "hal/servo.hpp"
#include "hal/gpio.hpp"

using namespace std::chrono_literals;

int main () {
	Servo rudder;
	Pin enable(SYSTEM_SERVO_ENB_PORT, SYSTEM_SERVO_ENB_PIN, true, true);
	if (!rudder.attach(RUDDER_PORT, RUDDER_PIN)) {
		std::cout << "Rudder failed to attach 1" << std::endl;
		return -1;
	}
	if (!rudder.isAttached())  {
		std::cout << "Rudder failed to attach 2" << std::endl;
		return -1;
	}
	for (double i = -100; i < 100.1; i += 1) {
		std::cout << "Setting rudder to " << std::to_string(i) << std::endl;
		rudder.write(i);
		std::cout << "Rudder set to " << std::to_string(rudder.read());
		std::cout << " or " << std::to_string(rudder.readMicroseconds()) << " us" << std::endl;
		std::this_thread::sleep_for(150ms);
	}
	
	for (double i = 100; i > -100.1; i -= 1) {
		std::cout << "Setting rudder to " << std::to_string(i) << std::endl;
		rudder.write(i);
		std::cout << "Rudder set to " << std::to_string(rudder.read());
		std::cout << " or " << std::to_string(rudder.readMicroseconds()) << " us" << std::endl;
		std::this_thread::sleep_for(150ms);
	}
	
	return 0;
}