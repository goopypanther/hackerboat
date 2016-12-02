/******************************************************************************
 * Hackerboat Beaglebone Throttle Functional Test program
 * throttle_test.cpp
 * This program is a functional test of the throttle subsystem
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
#include "hal/throttle.hpp"
#include "easylogging++.h"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	Throttle throttle;
	for (int i = throttle.getMinThrottle(); i <= throttle.getMaxThrottle(); i++) {
		std::cout << "Setting throttle to " << std::to_string(i) << std::endl;
		throttle.setThrottle(i);
		std::cout << "Throttle set to " << std::to_string(throttle.getThrottle()) << std::endl;
		std::this_thread::sleep_for(1500ms);
	}
	for (int i = throttle.getMaxThrottle(); i >= throttle.getMinThrottle(); i--) {
		std::cout << "Setting throttle to " << std::to_string(i) << std::endl;
		throttle.setThrottle(i);
		std::cout << "Throttle set to " << std::to_string(throttle.getThrottle()) << std::endl;
		std::this_thread::sleep_for(1500ms);
	}
	throttle.setThrottle(0);
	return 0;
}