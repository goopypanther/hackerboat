/******************************************************************************
 * Hackerboat Beaglebone Orientation Functional Test program
 * rc_test.cpp
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


using namespace std;

void runTestSet (OrientationInput *orient) {
	Orientation *data = orient->getOrientation();
	double pitch, roll, heading;
	bool valid;
	for (int i = 0; i < 120; i++) {
		//orient->lock.try_lock_for(100ms);
		pitch = data->pitch;
		roll = data->roll;
		heading = data->heading;
		valid = data->isValid();
		//orient->lock.unlock();
		cout << to_string(pitch) << "\t" << to_string(roll) << "\t"; 
		cout << to_string(heading) << "\t" << valid << endl;
		std::this_thread::sleep_for(500ms);
	}
}

int main () {
	OrientationInput orient(SensorOrientation::SENSOR_AXIS_X_UP);
	if (orient.begin() && orient.isValid()) {
		cout << "Initialization successful" << endl;
		cout << "Oriented with X axis up" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
		orient.setAxis(SensorOrientation::SENSOR_AXIS_X_DN);
		cout << "Oriented with X axis down" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
		orient.setAxis(SensorOrientation::SENSOR_AXIS_Y_UP);
		cout << "Oriented with Y axis up" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
		orient.setAxis(SensorOrientation::SENSOR_AXIS_Y_DN);
		cout << "Oriented with Y axis down" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
		orient.setAxis(SensorOrientation::SENSOR_AXIS_Z_UP);
		cout << "Oriented with Z axis up" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
		orient.setAxis(SensorOrientation::SENSOR_AXIS_Z_DN);
		cout << "Oriented with Z axis down" << endl;
		cout << "Pitch\tRoll\tHeading\tValid" << endl;
		runTestSet(&orient);
	} else {
		cout << "Initialization failed; exiting" << endl;
		return -1;
	}
	return 0;
}