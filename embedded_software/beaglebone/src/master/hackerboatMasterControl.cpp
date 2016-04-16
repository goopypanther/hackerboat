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
#include <BlackGPIO/BlackGPIO.h>

#include "stateStructTypes.hpp"
#include "boneState.hpp"
#include "arduinoState.hpp"
#include "logs.hpp"
#include "stateMachine.hpp"
#include "timer_utils.hpp"

#define CLOCKID CLOCK_REALTIME

using namespace BlackLib;

void inputBB (boneStateClass *state, arduinoStateClass *ard);
void outputBB (boneStateClass *state, arduinoStateClass *ard);

static logError *err = logError::instance();

int main (void) {
	BlackGPIO clockPin(gpioName::GPIO_38, direction::output, workingMode::FastMode);
	stateMachineBase *thisState, *lastState;
	boneStateClass myBoat;
	arduinoStateClass myArduino;
	timespec startTime, endTime, waitTime, frametime, framerun;
	
	logError::instance()->open(MAIN_LOGFILE);
	clockPin.setValue(digitalValue::low);
	
	thisState = new boneStartState(&myBoat, &myArduino);
	
	// initialize the frametime timespec
	frametime.tv_sec = 0;
	frametime.tv_nsec = FRAME_LEN_NS;
	norm_timespec(&frametime);
	
	
	for (;;) {
		clock_gettime(CLOCK_REALTIME, &startTime); 			// grab the time at the start of the frame
		clockPin.setValue(digitalValue::high);				// mark the start of the frame for debug
		inputBB(&myBoat, &myArduino);							// read all inputs
		lastState = thisState;								// store a pointer to the old state
		thisState = thisState->execute();					// run the current state
		if (thisState != lastState) delete lastState;		// if we have a new state, delete the old state object
		outputBB(&myBoat, &myArduino);						// write outputs
		clock_gettime(CLOCK_REALTIME, &endTime);			// get the time at the end of the frame 
		clockPin.setValue(digitalValue::low);				// mark the end of the frame for debug
		subtract_timespec(&endTime, &startTime, &framerun);	// calculate the duration of the frame 
		if (subtract_timespec(&frametime, &framerun, &waitTime)) {	// this returns false if the running frame time is longer than the specified frame time
			nanosleep(&waitTime, &waitTime);
		} else {
			logError::instance()->write("Master control", "Frame overrun");
		}
	}
}

void inputBB (boneStateClass *state, arduinoStateClass *ard) {
	ard->populate();
	state->getLastRecord();
	clock_gettime(CLOCK_REALTIME, &(state->uTime));
	if (!state->gps.getLastRecord()) {
		state->insertFault("No GNSS");
		state->setMode(boatModeEnum::FAULT);
	}
}

void outputBB (boneStateClass *state, arduinoStateClass *ard) {
	ard->heartbeat();
	ard->appendRecord();
	state->appendRecord();
}