/******************************************************************************
 * Hackerboat Beaglebone RC modes module
 * rcModes.hpp
 * This is the RC sub-modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef RCMODES_H
#define RCMODES_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"

class rcModeBaseClass : public stateMachineBaseClass<rcModeEnum, boatStateClass> {
};

class rcIdleMode : public rcModeBaseClass {
	public:
		rcIdleMode (boatStateClass& state, rcModeEnum last = NONE) : 
			_state(state), _thisMode(IDLE), _lastMode(last);
};

class rcRudderMode : public rcModeBaseClass {
	public:
		rcRudderMode (boatStateClass& state, rcModeEnum last = NONE) : 
			_state(state), _thisMode(RUDDER), _lastMode(last);
};

class rcCourseMode : public rcModeBaseClass {
	public:
		rcCourseMode (boatStateClass& state, rcModeEnum last = NONE) : 
			_state(state), _thisMode(COURSE), _lastMode(last);
};

#endif