/******************************************************************************
 * Hackerboat Beaglebone Master Control program
 * hackerboatMasterControl.cpp
 * This program is the core vessel control program
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
#include "boatState.hpp"
#include "boatModes.hpp"
#include "autoModes.hpp"
#include "rcModes.hpp"
#include "navModes.hpp"
#include "stateMachine.hpp"
#include "location.hpp"
#include "orientation.hpp"
#include "pid.hpp"
#include "json_utilities.hpp"
#include "aio-rest.hpp"

#include "hal/adcInput.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/orientationInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/servo.hpp"
#include "hal/throttle.hpp"
#include "waypoint.hpp"
#include "easylogging++.h"

#include "util.hpp"

#define ELPP_STL_LOGGING

INITIALIZE_EASYLOGGINGPP

//void inputBB (BoatState &state, long stepNum);
//void outputBB (BoatState &state, long stepNum);

int main (int argc, char **argv) {
	cout << "Starting up..." << std::endl;

    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	START_EASYLOGGINGPP(argc, argv);

	// system setup
	BoatState state;
	state.rc = new RCInput();
	state.adc = new ADCInput();
	state.health = new HealthMonitor(state.adc);
	state.gps = new GPSdInput();
	state.relays = RelayMap::instance();
	state.rudder = new Servo();
	state.throttle = new Throttle();
	state.orient = new OrientationInput(SensorOrientation::SENSOR_AXIS_Z_UP);

	// start the input threads
	if (!state.rudder->attach(RUDDER_PORT, RUDDER_PIN)) {
		LOG(FATAL) << "Rudder failed to attach";
		return -1;
	}
	if (!state.orient->begin()) {
		LOG(FATAL)  << "Orientation subsystem failed to start";
		return -1;
	}
	if (!state.rc->begin()) {
		LOG(FATAL)  << "RC subsystem failed to start";
		return -1;
	}
	if (!state.gps->begin()) {
		LOG(FATAL)  << "GPS subsystem failed to start";
		return -1;
	}
	if (!state.adc->begin()) {
		LOG(FATAL)  << "ADC subsystem failed to start";
		return -1;
	}
	state.relays->init();
	
	// AIO REST setup
	AIO_Rest myrest(&state);
	cout << "Creating publishing map..." << endl;
	PubFuncMap *mypubmap = new PubFuncMap {	{"SpeedLocation", new pub_SpeedLocation(&state, &myrest)},
											{"Mode", new pub_Mode(&state, &myrest)},
											{"MagneticHeading", new pub_MagHeading(&state, &myrest)},
											{"GPSCourse", new pub_GPSCourse(&state, &myrest)},
											{"BatteryVoltage", new pub_BatteryVoltage(&state, &myrest)},
											{"RudderPosition", new pub_RudderPosition(&state, &myrest)},
											{"ThrottlePosition", new pub_ThrottlePosition(&state, &myrest)},
											{"FaultString", new pub_FaultString(&state, &myrest)},
											{"Waypoint", new pub_Waypoint(&state, &myrest)} };
	cout << "Publishing map created..." << endl;
	SubFuncMap *mysubmap = new SubFuncMap {{"Command", new sub_Command(&state, &myrest)}};
	cout << "Subscription map created..." << endl;
	myrest.setPubFuncMap(mypubmap);
	myrest.setSubFuncMap(mysubmap);
	myrest.begin();

	// create the boat mode
	BoatModeBase *mode, *oldmode = {0};
	mode = BoatModeBase::factory(state, BoatModeEnum::START);

	// load KML file
	if (!state.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/2017Mar25.kml")) {
		LOG(ERROR)  << "Waypoint list failed to load";
		cout << "KML failed to load" << endl;
		//return -1;
	}

	cout << "All configured -- entering state" << std::endl;
	LOG(INFO) << ",CSV," << state.getCSVheaders();

	// run the boat
	for (;;) {
		// read inputs
		state.lastFix.copy(state.gps->getFix());
		state.health->readHealth();

		// run the state
		auto endtime = std::chrono::system_clock::now() + 100ms;
		if (state.commandCnt()) {
			cout << to_string(state.commandCnt()) << " commands in the queue" << endl;
			cout << to_string(state.executeCmds(0)) << " commands successfully executed" << endl;
		}
		oldmode = mode;
		mode = mode->execute();
		if (mode != oldmode) REMOVE(oldmode);		
		LOG_EVERY_N(5, INFO) << ",CSV," << state.getCSV();
		std::this_thread::sleep_until(endtime);
	}

	return 0;

}
