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
 

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include "hal/servo.hpp"
#include "hal/gpio.hpp"
#include "easylogging++.h"
#include "configuration.hpp"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

using namespace std::chrono_literals;

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	Servo rudder;
	Pin enable(Conf::get()->servoEnbPort(), Conf::get()->servoEnbPin(), true, true);
	if (!rudder.attach(Conf::get()->rudderPort(), Conf::get()->rudderPin())) {
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