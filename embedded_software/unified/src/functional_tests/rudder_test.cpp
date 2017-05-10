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
#include "pid.hpp"
#include "boatState.hpp"
#include "hal/servo.hpp"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
    
    BoatState *me = new BoatState();
	me->rudder = new Servo();
	me->throttle = new Throttle();
	me->orient = new OrientationInput(SensorOrientation::SENSOR_AXIS_Z_UP);
    
	double targetHeading = 0;
	double in = 0, out = 0, setpoint = 0;
	Pin enable(Conf::get()->servoEnbPort(), Conf::get()->servoEnbPin(), true, true);
	PID *helm = new PID(&in, &out, &setpoint, 0, 0, 0, 0);
	helm->SetMode(AUTOMATIC);
	helm->SetControllerDirection(Conf::get()->rudderDir());
	helm->SetSampleTime(Conf::get()->rudderPeriod());
	helm->SetOutputLimits(Conf::get()->rudderMin(), 
						Conf::get()->rudderMax());
	helm->SetTunings(10,0,0);
	
	if (!me->rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin())) {
		std::cout << "Rudder failed to attach 1" << std::endl;
		return -1;
	}
	if (!me->rudder->isAttached())  {
		std::cout << "Rudder failed to attach 2" << std::endl;
		return -1;
	}
	if (me->orient->begin() && me->orient->isValid()) {
		cout << "Initialization successful" << endl;
		cout << "Oriented with Z axis up" << endl;
	} else {
		cout << "Initialization failed; exiting" << endl;
		return -1;
	}
	
	for (int i = 0; i < 100; i++) {
		double currentheading = me->orient->getOrientation()->makeTrue().heading;
		if (isfinite(currentheading)) targetHeading += currentheading;
		cout << ".";
		std::this_thread::sleep_for(100ms);
	}
	cout << endl;
	targetHeading = targetHeading/100;
	cout << "Target heading is " << to_string(targetHeading) << " degrees true " << endl;
	int count = 0;
	for (;;) {
		in = me->orient->getOrientation()->makeTrue().headingError(targetHeading);
		count++;
		LOG_EVERY_N(10, DEBUG) << "True Heading: " << me->orient->getOrientation()->makeTrue() 
								<< ", Target Course: " << targetHeading;
		helm->Compute();
		me->rudder->write(out);
		LOG_EVERY_N(10, DEBUG) << "Rudder command: " << to_string(out);
		std::this_thread::sleep_for(100ms);
		if (count > 9) {
			count = 0;
			cout << "True Heading: " << me->orient->getOrientation()->makeTrue().heading 
								<< "\tTarget Course: " << targetHeading
								<< "\tRudder command: " << to_string(out) << endl;
		}
	}
	
	return 0;
}
