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
#include "hal/throttle.hpp"

int main () {
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
	return 0;
}