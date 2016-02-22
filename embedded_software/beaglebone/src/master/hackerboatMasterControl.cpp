/******************************************************************************
 * Hackerboat Beaglebone Master Control program
 * hackerboatMasterControl.cpp
 * This program is the core vessel control program
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
#include "stateStructTypes.hpp"
#include "config.h"
#include "logs.hpp"
#include "navigation.hpp"
#include "location.hpp"

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

static void handler(int sig, siginfo_t *si, void *uc);
void input (boneStateClass *state);
void output (boneStateClass *state);

static bool timerFlag = true;

int main (void) {
	boneStateClass myState(BONE_LOG_DB_FILE, strlen(BONE_LOG_DB_FILE));
	stateMachineBase *thisState, *lastState;
	timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    sigset_t mask;
    struct sigaction sa;
	
	logError::instance()->open(MAIN_LOGFILE);
	
	// Establish the handler for the timer signal
	sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
		logError::instance()->write("main control", "sigaction");
		exit(EXIT_FAILURE);
	}
	
	// Create the timer
	sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCKID, &sev, &timerid) == -1) {
		logError::instance()->write("main control", "timer_create");
		exit(EXIT_FAILURE);
	}
	
	// Start the timer
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = FRAME_LEN_NS;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		logError::instance()->write("main control", "timer_settime");
		exit(EXIT_FAILURE);
	}
	
	thisState = new boneStartState(&myState);
	
	for (;;) {
		while (!timerFlag);								// wait for the timer flag to go true
		input(&myState);
		lastState = thisState;
		thisState = thisState->execute();
		if (thisState == lastState) delete lastState;	// if we have a new state, delete the old object
		output(&myState);
	}
}

static void handler(int sig, siginfo_t *si, void *uc) {
	if (!timerFlag) {
		timerFlag = true;
	} else {
		logError::instance()->write("main control", "frame overrun");
	}
	signal(sig, SIG_IGN);
}