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
 

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include "hal/gpsdInput.hpp"
#include "gps.hpp"
#include "easylogging++.h"
#include "mqtt.hpp"

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
	MQTT mymqtt(&me);
	PubFuncMap *mypubmap = new PubFuncMap {{"SpeedLocation", PubFunc(MQTT::pub_SpeedLocation)},
							{"Mode", PubFunc(MQTT::pub_Mode)},
							{"Bearing", PubFunc(MQTT::pub_Bearing)},
							{"BatteryVoltage", PubFunc(MQTT::pub_BatteryVoltage)},
							{"RudderPosition", PubFunc(MQTT::pub_RudderPosition)},
							{"ThrottlePosition", PubFunc(MQTT::pub_ThrottlePosition)},
							{"PID", PubFunc(MQTT::pub_PID_K)},
							{"Course", PubFunc(MQTT::pub_Course)}};
	cout << "Setting up publishing map...";
	mymqtt.setPubFuncMap(mypubmap);
	cout << "Connecting... " << to_string(mymqtt.connect());
	for (int x = 0; x < 20; x++) {
		cout << "Calling publishNext()... " << to_string(x);
		mymqtt.publishNext();
	}
	cout << "Calling publishAll()...";
	mymqtt.publishAll();
	
	return 0;
}