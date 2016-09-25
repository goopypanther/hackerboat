
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
#include "hal/throttle.hpp"
#include "hal/servo.hpp"
#include "hal/orientationInput.hpp"
#include "hal/relay.hpp"
#include "hal/RCinput.hpp"

using namespace std;

int main () {
	bool startState = false;
	Throttle throttle;
	Servo rudder;
	Pin servoenable(SYSTEM_SERVO_ENB_PORT, SYSTEM_SERVO_ENB_PIN, true, true);
	Pin startbutton(SYSTEM_ARM_INPUT_PORT, SYSTEM_ARM_INPUT_PIN, false, false);
	Pin stopbutton(SYSTEM_DISARM_INPUT_PORT, SYSTEM_DISARM_INPUT_PIN, false, false);
	RCInput rc;
	OrientationInput orient(SensorOrientation::SENSOR_AXIS_Z_UP);
	RelayMap *relays = RelayMap::instance();
	ofstream logfile;
	
	throttle.setThrottle(0);
	if (!rudder.attach(RUDDER_PORT, RUDDER_PIN)) {
		std::cout << "Rudder failed to attach 1" << std::endl;
		return -1;
	}
	if (!orient.begin()) {
		cout << "Orientation subsystem failed to start" << endl;
		return -1;
	}
	if (!rc.begin()) {
		cout << "RC subsystem failed to start" << endl;
		return -1;
	}
	logfile.open("rcops.log");
	if (!logfile.is_open()) {
		cout << "Log file did not open" << endl;
		return -1;
	}
	cout << "Setup completed!" << endl;
	while (1) {
		if (!startbutton.get()) startState = true;
		if (!stopbutton.get()) startState = false;
		rudder.write(rc.getRudder());
		if (startState && (rc.getChannel(RC_AUTO_SWITCH) > RC_MIDDLE_POSN) && !(rc.isFailSafe())) {
			throttle.setThrottle(rc.getThrottle());
		} else throttle.setThrottle(0);
		if (rc.getChannel(RC_HORN_SWITCH) > RC_MIDDLE_POSN) {
			relays->get("HORN").set();
		} else {
			relays->get("HORN").clear();
		}
		string csv = HackerboatState::packTime(chrono::system_clock::now());
		csv += "," + to_string(startbutton.get());
		csv += "," + to_string(stopbutton.get());
		csv += "," + startState;
		csv += "," + to_string(rc.getRudder());
		csv += "," + to_string(rudder.readMicroseconds());
		csv += "," + to_string(rc.getThrottle());
		csv += "," + to_string(rc.getChannel(RC_AUTO_SWITCH));
		csv += "," + to_string(rc.getChannel(RC_HORN_SWITCH));
		csv += "," + rc.isFailSafe();
		csv += "," + to_string(orient.getOrientation()->heading);
		cout << csv << endl;
		logfile << csv << endl;
		std::this_thread::sleep_for(50ms);
	}
	logfile.close();
	return 0;
}