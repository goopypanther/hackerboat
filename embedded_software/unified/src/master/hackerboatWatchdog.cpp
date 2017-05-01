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
#include "hal/throttle.hpp"
extern "C" {
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
}

#include "easylogging++.h"
#include "util.hpp"

#define ELPP_STL_LOGGING

INITIALIZE_EASYLOGGINGPP

using namespace std::chrono;

bool killRelays () {
	RelayMap *relays = RelayMap::instance();
	bool result = true;
	result &= relays->get("DIR").clear();
	result &= relays->get("RED").clear();
	result &= relays->get("WHT").clear();
	result &= relays->get("YLW").clear();
	result &= relays->get("REDWHT").clear();
	result &= relays->get("YLWWHT").clear();
	return result;
}

int main (int argc, char **argv) {
	std::string wdFilePath;
	struct stat wdFileStatus;

    // Load configuration from file
    el::Configurations conf("/home/debian/hackerboat/embedded_software/unified/setup/log.conf");
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
	//START_EASYLOGGINGPP(argc, argv);

	cerr << "Starting watchdog..." << std::endl;

	// start up the relays
	RelayMap *relays = RelayMap::instance();
	relays->init();

	// Determine the watchdog file
	if (argc > 2) {
		wdFilePath = argv[1];
	} else {
		wdFilePath = WD_DEFAULT_FILE;
	}
	cerr << "Watchdog file is: " << wdFilePath << endl;
	if (stat(wdFilePath.c_str(), &wdFileStatus)) {
		cerr << "Unable to open " << wdFilePath << " errno: " << errno << std::endl;
		return -1;
	}

	// run the watchdog
	for (;;) {
		// Check the last file modification time. Turn off relays if file can't be found
		if (stat(wdFilePath.c_str(), &wdFileStatus)) {
			cerr << "Unable to open " << wdFilePath << " errno: " << errno << std::endl;
			killRelays();
		} else {
			// If time since last modification greater than timeout, turn off all relays
			system_clock::time_point lastmod(duration_cast<system_clock::duration>(
												seconds{wdFileStatus.st_mtim.tv_sec} + 
												nanoseconds{wdFileStatus.st_mtim.tv_nsec}));
			if ((system_clock::now() - lastmod) > WD_TIMEOUT) {
				cerr << "Watchdog fired" << std::endl;
				killRelays();
			}
		}
		std::this_thread::sleep_for(1s);
	}

	return 0;
}
