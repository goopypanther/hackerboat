/******************************************************************************
 * Hackerboat Beaglebone State Machine module
 * hackerboatStateMachine.c
 * This module handles incoming REST requests
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>
#include "logs.h"
#include "dbReadWrite.h"
#include "config.h"

// function definitions

void initIO (void);
void initBoat (boneVector *vec);

// state functions
boneState executeStart		(boneVector *vec, boneState last);
boneState executeSelfTest	(boneVector *vec, boneState last);
boneState executeDisarmed	(boneVector *vec, boneState last);
boneState executeFault		(boneVector *vec, boneState last);
boneState executeArmed		(boneVector *vec, boneState last);
boneState executeManual		(boneVector *vec, boneState last);
boneState executeWaypoint	(boneVector *vec, boneState last);
boneState executeNoSignal	(boneVector *vec, boneState last);
boneState executeArmedTest	(boneVector *vec, boneState last);

// Main function
int main (void) {
	boneVector		vec;
	timespec		thisTime, lastTime, sleepTime, remTime;
	long 			deltaTime;
	boneState		thisState = BONE_UNKNOWN;
	boneState		lastState = BONE_UNKNOWN;
	char[255]		errString;
	
	initIO();
	initBoat(&vec);
	clock_gettime(CLOCK_MONOTONIC, &thisTime);
	
	for (;;) {
		// get the current time
		clock_gettime(CLOCK_MONOTONIC, &thisTime);
		// calculate how long has elapsed since the start of the last frame
		if (thisTime.tv_nsec > lastTime.tv_nsec) {
			deltaTime = thisTime.tv_nsec - lastTime.tv_nsec;
		} else {
			deltaTime = thisTime.tv_nsec - lastTime.tv_nsec + 1e9;
		}
		
		// sleep until the next frame is supposed to start
		if (deltaTime < FRAME_LEN_NS) {
			sleepTime.tv_sec = 0;
			sleepTime.tv_nsec = FRAME_LEN_NS - deltaTime;
			if (nanosleep(&sleepTime, &remTime)) continue;	// if nanosleep returns nonzero, it means we got interrupted, so try again
			else clock_gettime(CLOCK_MONOTONIC, &thisTime);		
			lastTime = thisTime;
		}
		
		if (getBoneVector(&vec, -1, 1)) {
			logError("StateMachine", "Failed to write new bone state vector to database");
			continue;				// if we can't read the database, skip this frame
		}
		
		vec.uTime = ((thisTime.tv_sec * 1000000) + (long)(thisTime.tv_nsec/1000)); // set the start of the frame
		switch (vec.state) {
			case BONE_START:
				thisState = vec.state;
				vec.state = executeStart(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_SELFTEST:
				thisState = vec.state;
				vec.state = executeSelfTest(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_DISARMED:
				thisState = vec.state;
				vec.state = executeDisarmed(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_FAULT:
				thisState = vec.state;
				vec.state = executeFault(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_ARMED:
				thisState = vec.state;
				vec.state = executeArmed(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_MANUAL:
				thisState = vec.state;
				vec.state = executeManual(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_WAYPOINT:
				thisState = vec.state;
				vec.state = executeWaypoint(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_NOSIGNAL:
				thisState = vec.state;
				vec.state = executeNoSignal(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_RETURN:
				thisState = vec.state;
				vec.state = executeReturn(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_ARMEDTEST:
				thisState = vec.state;
				vec.state = executeArmedTest(&vec, lastState);
				lastState = thisState;
				break;
			case BONE_NONE:
			case default:
				sprintf(errString, "Invalid state detected: %d", vec.state);
				logError("StateMachine", errString);
				vec.state = BONE_SELFTEST;
				lastState = BONE_NONE;
				thisState = BONE_NONE;
				break;
		}
		
		if (writeBoneVector(&vec)) {
			logError("StateMachine", "Failed to write new bone state vector to database");
		}
	}	
}

void initIO (void) {
	// initialize Arduino serial ports
}

void initBoat (boneVector *vec) {
	timespec thisTime;
	clock_gettime(CLOCK_MONOTONIC, &thisTime);
	
	vec->sequenceNum 		= -1;
	vec->uTime 				= ((thisTime.tv_sec * 1000000) + (long)(thisTime.tv_nsec/1000));
	vec->state				= BONE_START;
	vec->command			= BONE_NONE;
	vec->arduinoState		= BOAT_NONE;
	
	
}

boneState executeStart		(boneVector *vec, boneState last) {
}

boneState executeSelfTest	(boneVector *vec, boneState last) {
}

boneState executeDisarmed	(boneVector *vec, boneState last) {
}

boneState executeFault		(boneVector *vec, boneState last) {
}

boneState executeArmed		(boneVector *vec, boneState last) {
}

boneState executeManual		(boneVector *vec, boneState last) {
}

boneState executeWaypoint	(boneVector *vec, boneState last) {
}

boneState executeNoSignal	(boneVector *vec, boneState last) {
}

boneState executeArmedTest	(boneVector *vec, boneState last) {
}
