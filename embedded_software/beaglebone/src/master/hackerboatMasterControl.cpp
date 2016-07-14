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
 
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "stateStructTypes.hpp"
#include "boneState.hpp"
#include "arduinoState.hpp"
#include "logs.hpp"
#include "stateMachine.hpp"
#include "timer_utils.hpp"

#define CLOCKID CLOCK_REALTIME

void inputBB (boneStateClass *state, arduinoStateClass *ard, long stepNum);
void outputBB (boneStateClass *state, arduinoStateClass *ard, long stepNum);

static logError *err = logError::instance();

int main (void) {
	long stepNum = 0;
	stateMachineBase *thisState, *lastState;
	boneStateClass myBoat;
	arduinoStateClass myArduino;
	timespec startTime, endTime, waitTime, frametime, framerun;
	
	logError::instance()->open(MAIN_LOGFILE);
	
	thisState = new boneStartState(&myBoat, &myArduino);
	
	// initialize the frametime timespec
	frametime.tv_sec = 0;
	frametime.tv_nsec = FRAME_LEN_NS;
	norm_timespec(&frametime);
	//printf("Frametime is %ld.%ld\n", (long)frametime.tv_sec, frametime.tv_nsec);
	
	
	for (;;) {
		clock_gettime(CLOCK_REALTIME, &startTime); 			// grab the time at the start of the frame
		inputBB(&myBoat, &myArduino, stepNum);							// read all inputs
		lastState = thisState;								// store a pointer to the old state
		thisState = thisState->execute();					// run the current state
		if (thisState != lastState) delete lastState;		// if we have a new state, delete the old state object
		outputBB(&myBoat, &myArduino, stepNum);						// write outputs
		clock_gettime(CLOCK_REALTIME, &endTime);			// get the time at the end of the frame 
		subtract_timespec(&endTime, &startTime, &framerun);	// calculate the duration of the frame 
		//printf("Start time is %ld.%ld\n", (long)startTime.tv_sec, startTime.tv_nsec);
		//printf("End time is %ld.%ld\n", (long)endTime.tv_sec, endTime.tv_nsec);
		//printf("Frame run time is %ld.%ld\n", (long)framerun.tv_sec, framerun.tv_nsec);
		if (subtract_timespec(&frametime, &framerun, &waitTime)) {	// this returns false if the running frame time is longer than the specified frame time
			//printf("Sleep time is %ld.%ld\n", (long)waitTime.tv_sec, waitTime.tv_nsec);
			nanosleep(&waitTime, &waitTime);
		} else {
			logError::instance()->write("Master control", "Frame overrun");
		}
		stepNum++;
	}
}

void inputBB (boneStateClass *state, arduinoStateClass *ard, long stepNum) {
	ard->getLastRecord();
	state->getLastRecord();
	if (stepNum % 10) {
		ard->corePopulate();
	} else {
		ard->populate();
	}
	clock_gettime(CLOCK_REALTIME, &(state->uTime));
	if (!state->gps.getLastRecord()) {
		state->insertFault("No GNSS");
		state->setMode(boatModeEnum::FAULT);
	}
}

void outputBB (boneStateClass *state, arduinoStateClass *ard, long stepNum) {
	ard->writeBoatMode(state->mode);
	ard->heartbeat();
	ard->appendRecord();
	state->appendRecord();
}