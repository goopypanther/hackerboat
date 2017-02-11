/******************************************************************************
 * Hackerboat Beaglebone Relay Functional Test program
 * relay_test.cpp
 * This program is a functional test of the Relay subsystem
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
#include <jansson.h>
#include "hal/relay.hpp"
#include "easylogging++.h"

#define ELPP_STL_LOGGING 

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char **argv) {
	START_EASYLOGGINGPP(argc, argv);
    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	RelayMap* relays = RelayMap::instance();
	cout << "Initializing relays...";
	if (relays->init()) {
		cout << " Relays initialized!" << endl;
	} else return -1;
	cout << "Setting relays in sequence..." << endl;
	for (auto &r : *(relays->getmap())) {
		cout << "Setting " << r.first << endl;
		r.second.set();
		//if (r.second.pack()) cout << "Successfully packed" << endl;
		cout << string(json_dumps(r.second.pack(), JSON_ENSURE_ASCII)) << endl;
		std::this_thread::sleep_for(1500ms);
		cout << "Clearing " << r.first << endl;
		r.second.clear();
		cout << string(json_dumps(r.second.pack(), JSON_ENSURE_ASCII)) << endl;
	}
	return 0;
}