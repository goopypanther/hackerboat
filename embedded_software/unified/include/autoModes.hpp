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

class autoModeBaseClass : public stateMachineBaseClass<autoModeEnum, boatStateClass> {
};

class autoIdleMode : public autoModeBaseClass {
	public:
		autoIdleMode (boatStateClass& state, autoModeEnum last = NONE) : 
			_state(state), _thisMode(IDLE), _lastMode(last);
};

class autoWaypointMode : public autoModeBaseClass {
	public:
		autoWaypointMode (boatStateClass& state, autoModeEnum last = NONE) : 
			_state(state), _thisMode(WAYPOINT), _lastMode(last);
};

class autoReturnMode : public autoModeBaseClass {
	public:
		autoReturnMode (boatStateClass& state, autoModeEnum last = NONE) : 
			_state(state), _thisMode(RETURN), _lastMode(last);
};

class autoAnchorMode : public autoModeBaseClass {
	public:
		autoAnchorMode (boatStateClass& state, autoModeEnum last = NONE) : 
			_state(state), _thisMode(ANCHOR), _lastMode(last);
};

#endif