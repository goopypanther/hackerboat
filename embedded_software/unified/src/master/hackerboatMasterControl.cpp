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

#include "hal/adcInput.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/orientationInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/servo.hpp"
#include "hal/throttle.hpp"
#include "waypoint.hpp"
#include "easylogging++.h"

#define ELPP_STL_LOGGING

INITIALIZE_EASYLOGGINGPP


//void inputBB (BoatState &state, long stepNum);
//void outputBB (BoatState &state, long stepNum);

int main (int argc, char **argv) {
	cout << "Starting up..." << std::endl;
	// command file path
	std::string cmdfilepath = "/home/debian/hackerboat/embedded_software/unified/ctrl/command.json";
	std::ifstream cmdin;

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

	// create the boat mode
	BoatModeBase *mode, *oldmode;
	mode = BoatModeBase::factory(state, BoatModeEnum::START);
	oldmode = mode;

	cout << "All configured -- entering state" << std::endl;

	// run the boat
	for (;;) {
		// read inputs
		state.lastFix = *state.gps->getFix();
		state.health->readHealth();

		// look for commands in the command file
		// this is a thoroughly fugly hack to make up for the fact that MQTT is not up and running yet
		if (!access(cmdfilepath.c_str(), R_OK)) {
			std::string line;
			bool execflag = false;
			json_error_t err;
			cmdin.open(cmdfilepath);
			if (cmdin.is_open()) {
				while(cmdin.good() && !cmdin.eof()) {									// iterate through the file
					std::getline(cmdin, line);											// grab a line
					json_t *jsonin = json_loads(line.c_str(), 0, &err);					// try to parse JSON out of it
					if (jsonin) {
						std::string cmdname;
						if (::parse(json_object_get(jsonin, "Command"), &cmdname)) {	// check that we have a command name on this line
							execflag = true;											// set the execflag true so we execute the command
							json_t *cmdargs = json_object_get(jsonin, "Argument");		// grab the arguments, if any
							state.pushCmd(cmdname, cmdargs);							// add the command to the queue
							if (cmdargs) json_decref(cmdargs);							// clean up the json objects
							json_decref(jsonin);
						}
					}
				}
				cmdin.close();
				// we scrub the file once we've read it
				std::ofstream cmdout;
				cmdout.open(cmdfilepath, std::ofstream::trunc);		// open the file and discard the contents
				if (cmdout.is_open()) cmdout << "";					// overwrite the contents of the file if we opened it
				cmdout.close();
			}
			if (execflag) {
				state.executeCmds();
				execflag = false;
			}
		}

		// run the state
		auto endtime = std::chrono::system_clock::now() + 100ms;
		oldmode = mode;
		mode = mode->execute();
		if (mode != oldmode) delete oldmode;
		std::this_thread::sleep_until(endtime);
	}

	return 0;

}
