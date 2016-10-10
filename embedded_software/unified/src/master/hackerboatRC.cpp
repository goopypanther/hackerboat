
#include "hal/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include "orientation.hpp"
#include "enumdefs.hpp"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"
#include "pid.hpp"
#include "orientation.hpp"
#include "hal/throttle.hpp"
#include "hal/servo.hpp"
#include "hal/orientationInput.hpp"
#include "hal/relay.hpp"
#include "hal/RCinput.hpp"

using namespace std;

int main () {
	bool startState = false;
	
	ofstream logfile;
	
	BoatState state;
	state.rc = new RCInput();
	state.adc = new ADCInput();
	state.health = new HealthMonitor(state.adc);
	state.gps = new GPSdInput();
	state.relays = RelayMap::instance();
	state.rudder = new Servo();
	state.throttle = new Throttle();
	state.orient = new OrientationInput(SensorOrientation::SENSOR_AXIS_Z_UP);
	state.servoEnable.set();
	
	double courseError, rudderCommand;
	double errorSetpoint = 0.0;
	
	PID rudderpid (&courseError, &rudderCommand, &errorSetpoint, 0.0, 0.0, 0.0, REVERSE);
	rudderpid.SetMode(AUTOMATIC);
	rudderpid.SetOutputLimits(-100.0, 100.0);
	rudderpid.SetSampleTime(50);
	
	state.throttle->setThrottle(0);
	if (!state.rudder->attach(RUDDER_PORT, RUDDER_PIN)) {
		std::cout << "Rudder failed to attach 1" << std::endl;
		return -1;
	}
	if (!state.orient->begin()) {
		cout << "Orientation subsystem failed to start" << endl;
		return -1;
	}
	if (!state.rc->begin()) {
		cout << "RC subsystem failed to start" << endl;
		return -1;
	}
	if (!state.gps->begin()) {
		cout << "GPS subsystem failed to start" << endl;
		return -1;
	}
	if (!state.adc->begin()) {
		cout << "ADC subsystem failed to start" << endl;
		return -1;
	}
	logfile.open("rcops.log");
	if (!logfile.is_open()) {
		cout << "Log file did not open" << endl;
		return -1;
	}
	logfile << state.getCSVheaders();
	cout << "Setup completed!" << endl;
	while (1) {
		state.recordTime = std::chrono::system_clock::now();
		if (!state.armInput.get()) startState = true;
		if (!state.disarmInput.get()) startState = false;
	
		if (startState && (state.rc->getChannel(RC_AUTO_SWITCH) > RC_MIDDLE_POSN) && !(state.rc->isFailSafe())) {
			state.throttle->setThrottle(state.rc->getThrottle());
		} else state.throttle->setThrottle(0);
	
		if (state.rc->getMode() == RCModeEnum::RUDDER) {
			state.rudder->write(state.rc->getRudder());
		} else if (state.rc->getMode() == RCModeEnum::COURSE) {
			get<0>(state.K) = RCInput::map(static_cast<double>(state.rc->getChannel(2)), RC_MIN, RC_MAX, 0.0, 100.0);
			get<1>(state.K) = RCInput::map(static_cast<double>(state.rc->getChannel(1)), RC_MIN, RC_MAX, 0.0, 10.0);
			Orientation *heading = state.orient->getOrientation();
			courseError = heading->headingError(state.rc->getCourse());
			rudderpid.SetTunings(state.K);
			rudderpid.Compute();
			state.rudder->write(rudderCommand);
		} else {
			state.throttle->setThrottle(0);
		}
		
		if (state.rc->getChannel(RC_HORN_SWITCH) > RC_MIDDLE_POSN) {
			state.relays->get("HORN").set();
		} else {
			state.relays->get("HORN").clear();
		}
		string csv = state.getCSV();
		cout << csv << "," << to_string(get<0>(state.K));
		cout << "," << to_string(get<1>(state.K)) << ",";
		cout << to_string(state.rc->getCourse()) <<  endl;
		logfile << csv << "," << to_string(get<0>(state.K));
		logfile << "," << to_string(get<1>(state.K)) << ",";
		logfile << to_string(state.rc->getCourse()) << endl;
		std::this_thread::sleep_for(25ms);
	}
	logfile.close();
	return 0;
}
