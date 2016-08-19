/******************************************************************************
 * Hackerboat Beaglebone navigation modes module
 * navModes.hpp
 * This is the navigation sub-modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef NAVMODES_H
#define NAVMODES_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"
#include "rcMode.hpp"
#include "autoMode.hpp"

class navModeBaseClass : public stateMachineBaseClass<navigationModeEnum, boatStateClass> {
};

class navIdleMode : public navModeBaseClass {
	public:
		navIdleMode (boatStateClass& state, navigationModeEnum last = NONE) : 
			_state(state), _thisMode(IDLE), _lastMode(last);
};

class navFaultMode : public navModeBaseClass {
	public:
		navFaultMode (boatStateClass& state, navigationModeEnum last = NONE) : 
			_state(state), _thisMode(FAULT), _lastMode(last);
};

class navRCMode : public navModeBaseClass {
	public:
		navRCMode (boatStateClass& state, rcModeEnum _submode = IDLE, navigationModeEnum last = NONE) : 
			_state(state), _thisMode(RC), _lastMode(last);
		rcModeBaseClass& getRCMode () {return _rcMode;};
	private:
		rcModeBaseClass& _rcMode;
};

class navAutoMode : public navModeBaseClass {
	public:
		navAutoMode (boatStateClass& state, autoModeEnum _submode = IDLE, navigationModeEnum last = NONE) : 
			_state(state), _thisMode(AUTONOMOUS), _lastMode(last);
		autoModeBaseClass& getAutoMode () {return _autoMode;};
	private:
		autoModeBaseClass& _autoMode;
};

#endif