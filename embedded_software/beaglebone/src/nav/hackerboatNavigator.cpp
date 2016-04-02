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
#include <BlackGPIO/BlackGPIO.h>
#include "stateStructTypes.hpp"
#include "config.h"
#include "logs.hpp"
#include "navigation.hpp"
#include "location.hpp"
#include "boneState.hpp"
#include "sqliteStorage.hpp"

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

int initNav(navigatorBase *nav);
static void handler(int sig, siginfo_t *si, void *uc);

static bool timerFlag = true;

int main (void) {{
	BlackGPIO clockPin(GPIO_39, output, FastMode);
	navigatorBase *navInf;
	timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    sigset_t mask;
    struct sigaction sa;
	
	logError::instance()->open(NAV_LOGFILE);	// open up the logfile
	int navCount = initNav(navInf);					// initialize the list of nav sources
	clockPin.setValue(low);
	
	// Establish the handler for the timer signal
	sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
		logError::instance()->write("nav process", "sigaction");
		exit(EXIT_FAILURE);
	}
	
	// Create the timer
	sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCKID, &sev, &timerid) == -1) {
		logError::instance()->write("nav process", "timer_create");
		exit(EXIT_FAILURE);
	}
	
	// Start the timer
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = FRAME_LEN_NS;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		logError::instance()->write("nav process", "timer_settime");
		exit(EXIT_FAILURE);
	}
	
	for (;;) {
		while (!timerFlag);			// wait for the timer flag to go true
		clockPin.setValue(high);
		navClass nav;
		boneStateClass boat;
		boat.getLastRecord();
		if (nav.getLastRecord()) {
			if (nav.isValid() && boat.isValid()) {
				nav.clearVectors();
				for (uint16_t i = 0; i < navCount; i++) {
					if (navInf[i].isValid()) {
						nav.appendVector(navInf[i].calc());
					} 
				}
			} else {
				logError::instance()->write("nav process", "Fetched record is invalid");
			}
			nav.calc(boat.waypointStrengthMax);
			nav.writeRecord();
		} else {
			logError::instance()->write("nav process", "Failed to get last nav record");
		}
		timerFlag = false;		// mark that we're done with the frame
		clockPin.setValue(low);
	}
}

int initNav(navigatorBase *nav) {
	nav = NULL;
	return 0;
}

static void handler(int sig, siginfo_t *si, void *uc) {
	if (!timerFlag) {
		timerFlag = true;
	} else {
		logError::instance()->write("nav process", "frame overrun");
	}
	signal(sig, SIG_IGN);
}
