/******************************************************************************
 * Hackerboat Beaglebone RC Functional Test program
 * rc_test.cpp
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
#include <iomanip>
#include "hal/RCinput.hpp"
#include "boatState.hpp"

using namespace std;

int main () {
	RCInput rc;
	cout << "Starting RC subsystem: " << rc.begin() << endl;
	cout << "Throttle\tRudder\t\tCourse\t\tMode";
	cout << "\tCH0\tCH1\tCH2\tCH3\tCH4\tCH5\tCH6\tCH7\tCH8\tCH9\tCH10";
	cout << "\tCH11\tCH12\tCH13\tCH14\tCH15\tCH16\tCH17\tFailSafe" << endl;
	while (1) {
		cout << to_string(rc.getThrottle());
		cout << "\t\t" << to_string(rc.getRudder());
		cout << "\t" << to_string(rc.getCourse());
		cout << "\t" << BoatState::rcModeNames.get(rc.getMode());
		for (int i = 0; i < 18; i++) {
			cout << "\t" << to_string(rc.getChannel(i));
		}
		cout << "\t" << rc.isFailSafe();
		cout << endl;
		std::this_thread::sleep_for(500ms);
	}
	return 0;
}