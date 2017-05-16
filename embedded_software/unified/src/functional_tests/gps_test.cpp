/******************************************************************************
 * Hackerboat Beaglebone GPS Functional Test program
 * gps_test.cpp
 * This program is a functional test of the RC subsystem
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
#include "hal/gpsdInput.hpp"
#include "gps.hpp"
#include "easylogging++.h"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	GPSdInput gps;
	if (gps.begin()) {
		while(1) {
			GPSFix fix;
			fix.copy(gps.getFix());
			cout << "Time: ";
			cout << HackerboatState::packTime(fix.gpsTime);
			cout << "\tMode: " << GPSFix::NMEAModeNames.get(fix.mode);
			cout << "\tLat: " << fix.fix.lat;
			cout << "\tLon: " << fix.fix.lon;
			cout << "\tValid: " << fix.isValid() << endl;
			std::this_thread::sleep_for(500ms);
		}
	} else {
		cout << "Connection failed";
	}
	return 0;
}
