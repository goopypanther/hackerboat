/******************************************************************************
 * Hackerboat Beaglebone Orientation Functional Test program
 * orientation_test.cpp
 * This program is a functional test of the orientation input subsystem
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
#include "orientation.hpp"
#include "hal/orientationInput.hpp"
#include "easylogging++.h"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

using namespace std;

void runTestSet (OrientationInput *orient) {
	Orientation *data = orient->getOrientation();
	double pitch, roll, heading;
	bool valid;
	for (int i = 0; i < 300; i++) {
		//orient->lock.try_lock_for(100ms);
		pitch = data->pitch;
		roll = data->roll;
		heading = data->heading;
		valid = data->isValid();
		//orient->lock.unlock();
		cout << to_string(pitch) << "\t" << to_string(roll) << "\t"; 
		cout << to_string(heading) << "\t";
		map<char, double> mag = orient->compass.getMagData();
		map<char, double> acc = orient->compass.getAccelData();
		//cout << to_string(acc['x']) << "\t";
		//cout << to_string(acc['y']) << "\t";
		//cout << to_string(acc['z']) << "\t";
		cout << to_string(mag['x']) << "\t";
		cout << to_string(mag['y']) << "\t";
		cout << to_string(mag['z']) << "\t";
		cout << valid << endl;
		std::this_thread::sleep_for(250ms);
	}
}

void runMagExtrema (OrientationInput *orient) {
	int maxX = 0, minX = 0, maxY = 0, minY = 0, maxZ = 0, minZ = 0;
	cout << "maxX\tminX\tmaxY\tminY\tmaxZ\tminZ" << endl;
	for (int i = 0; i < 10000; i++) {
		map<char, int> mag = orient->compass.getRawMagData();
		if (mag['x'] > maxX) maxX = mag['x'];
		if (mag['x'] < minX) minX = mag['x'];
		if (mag['y'] > maxY) maxY = mag['y'];
		if (mag['y'] < minY) minY = mag['y'];
		if (mag['z'] > maxZ) maxZ = mag['z'];
		if (mag['z'] < minZ) minZ = mag['z'];
		if (!(i%50)) {
			cout << to_string(maxX) << "\t";
			cout << to_string(minX) << "\t";
			cout << to_string(maxY) << "\t";
			cout << to_string(minY) << "\t";
			cout << to_string(maxZ) << "\t";
			cout << to_string(minZ) << endl;
		}
		std::this_thread::sleep_for(10ms);
	}
}

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	OrientationInput orient(SensorOrientation::SENSOR_AXIS_Z_UP);
	if (orient.begin() && orient.isValid()) {
		cout << "Initialization successful" << endl;
		cout << "Oriented with Z axis up" << endl;
		cout << "Pitch\tRoll\tHeading\tAccelX\tAccelY\tAccelZ\tMagX\tMagY\tMagZ\tValid" << endl;
		runTestSet(&orient);
		//runMagExtrema(&orient);
	} else {
		cout << "Initialization failed; exiting" << endl;
		return -1;
	}
	return 0;
}