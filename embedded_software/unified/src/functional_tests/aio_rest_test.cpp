/******************************************************************************
 * Hackerboat Beaglebone MQTT Functional Test program
 * mqtt_test.cpp
 * This program is a functional test of the MQTT subsystem
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
#include "aio-rest.hpp"
#include "hal/orientationInput.hpp"
#include "hal/throttle.hpp"

#define ELPP_STL_LOGGING

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char **argv) {
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	START_EASYLOGGINGPP(argc, argv);

	// Setting up environment...
	BoatState me;
	me.throttle = new Throttle();
	me.orient = new OrientationInput(SensorOrientation::SENSOR_AXIS_Z_UP);
	me.orient->begin();
	AIO_Rest myrest(&me);
	cout << "Creating publishing map..." << endl;
	PubFuncMap *mypubmap = new PubFuncMap {	{"SpeedLocation", new pub_SpeedLocation(&me, &myrest)},
											{"Mode", new pub_Mode(&me, &myrest)},
											{"MagneticHeading", new pub_MagHeading(&me, &myrest)},
											{"GPSCourse", new pub_GPSCourse(&me, &myrest)},
											{"BatteryVoltage", new pub_BatteryVoltage(&me, &myrest)},
											{"RudderPosition", new pub_RudderPosition(&me, &myrest)},
											{"ThrottlePosition", new pub_ThrottlePosition(&me, &myrest)},
											{"FaultString", new pub_FaultString(&me, &myrest)}};
	cout << "Publishing map created..." << endl;
	SubFuncMap *mysubmap = new SubFuncMap {{"Command", new sub_Command(&me, &myrest)}};
	cout << "Subscription map created..." << endl;
	myrest.setPubFuncMap(mypubmap);
	myrest.setSubFuncMap(mysubmap);
	me.insertFault("testFault");
	for (int x = 0; x < 20; x++) {
		cout << "Calling publishNext()... " << to_string(myrest.publishNext()) << endl;
		if (x > 10) me.clearFaults();
		std::this_thread::sleep_for(500ms);
	}
	cout << "Starting threading test..." << endl;
	myrest.begin();

	for (int i = 0; i < 600; i++) {
		me.lastFix.fix.lat += 0.001;
		me.lastFix.fix.lon += 0.001;
		me.lastFix.speed += 0.01;
		if (me.commandCnt()) {
			cout << to_string(me.commandCnt()) << " commands in the queue" << endl;
			cout << to_string(me.executeCmds(0)) << " commands successfully executed" << endl;
		}
		std::this_thread::sleep_for(100ms);
	}
	myrest.kill();

	return 0;
}
