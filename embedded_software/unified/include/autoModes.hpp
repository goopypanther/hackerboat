/******************************************************************************
 * Hackerboat Beaglebone autonomous modes module
 * autoModes.hpp
 * This is the autonomous sub-modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef AUTOMODES_H
#define AUTOMODES_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"

class autoModeBaseClass : public stateMachineBaseClass<autoModeEnum, boatStateClass> {
	public:
		static autoModeBaseClass* autoModeFactory(boatStateClass& state, autoModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~autoModeBaseClass () {};
	protected:
		autoModeBaseClass (boatStateClass& state, autoModeEnum last, autoModeEnum thisMode) :
			stateMachineBaseClass<autoModeEnum, boatStateClass> (state, last, thisMode) {};
};

class autoIdleMode : public autoModeBaseClass {
	public:
		autoIdleMode (boatStateClass& state, autoModeEnum last = autoModeEnum::NONE) : 
			autoModeBaseClass(state, last, autoModeEnum::IDLE) {};
};

class autoWaypointMode : public autoModeBaseClass {
	public:
		autoWaypointMode (boatStateClass& state, autoModeEnum last = autoModeEnum::NONE) : 
			autoModeBaseClass(state, last, autoModeEnum::WAYPOINT) {}; 
};

class autoReturnMode : public autoModeBaseClass {
	public:
		autoReturnMode (boatStateClass& state, autoModeEnum last = autoModeEnum::NONE) : 
			autoModeBaseClass(state, last, autoModeEnum::RETURN) {}; 
};

class autoAnchorMode : public autoModeBaseClass {
	public:
		autoAnchorMode (boatStateClass& state, autoModeEnum last = autoModeEnum::NONE) : 
			autoModeBaseClass(state, last, autoModeEnum::ANCHOR) {}; 
};

#endif