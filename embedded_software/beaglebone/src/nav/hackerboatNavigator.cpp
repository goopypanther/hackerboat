/******************************************************************************
 * Hackerboat Beaglebone Navigator program
 * hackerboatNavigator.cpp
 * This program handles navigation
 * see the Hackerboat documentation for more details
 * timer code stolen shamelessly from the example at 
 * http://linux.die.net/man/2/timer_create
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <limits>
#include "stateStructTypes.hpp"
#include "config.h"
#include "logs.hpp"
#include "navigation.hpp"
#include "location.hpp"
#include "boneState.hpp"
#include "sqliteStorage.hpp"
#include "timer_utils.hpp"

#define CLOCKID CLOCK_REALTIME

int initNav(navigatorBase *nav);

int main (void) {
	navigatorBase *navInf;
	navClass nav;
	timespec startTime, endTime, waitTime, frametime, framerun;
	
	logError::instance()->open(NAV_LOGFILE);	// open up the logfile
	
	// initialize nav object & write some dummy stuff to the database if we fail
	int navCount = initNav(navInf);				// initialize the list of nav sources
	if (!nav.getLastRecord()) {
		
	}	

	// initialize the frametime timespec
	frametime.tv_sec = 0;
	frametime.tv_nsec = FRAME_LEN_NS;
	norm_timespec(&frametime);
	
	for (;;) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		
		boneStateClass boat;
		
		if (nav.getLastRecord() && boat.getLastRecord()) {
			if (nav.isValid() && boat.isValid()) {
				nav.clearVectors();
				for (uint16_t i = 0; i < navCount; i++) {
					if (navInf[i].isValid()) {
						nav.appendVector(navInf[i].calc());
					} 
				}
			} else {
				logError::instance()->write("Navigation", "Fetched record is invalid");
			}
			nav.calc(boat.waypointStrengthMax);
			nav.writeRecord();
		} else {
			logError::instance()->write("Navigation", "Failed to get last nav record");
		}
		clock_gettime(CLOCK_REALTIME, &endTime);			// get the time at the end of the frame 
		subtract_timespec(&endTime, &startTime, &framerun);	// calculate the duration of the frame 
		if (subtract_timespec(&frametime, &framerun, &waitTime)) {	// this returns false if the running frame time is longer than the specified frame time
			nanosleep(&waitTime, &waitTime);
		} else {
			logError::instance()->write("Navigation", "Frame overrun");
		}
	}
}

int initNav(navigatorBase *nav) {
	nav = NULL;
	return 0;
}

