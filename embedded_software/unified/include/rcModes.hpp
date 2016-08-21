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
	public:
		static rcModeBaseClass* rcModeFactory(boatStateClass& state, rcModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~rcModeBaseClass () {};
	protected:
		rcModeBaseClass (boatStateClass& state, rcModeEnum last, rcModeEnum thisMode) :
			stateMachineBaseClass<rcModeEnum, boatStateClass> (state, last, thisMode) {};
};

class rcIdleMode : public rcModeBaseClass {
	public:
		rcIdleMode (boatStateClass& state, rcModeEnum last = rcModeEnum::NONE) :
			rcModeBaseClass(state, last, rcModeEnum::IDLE) {};
};

class rcRudderMode : public rcModeBaseClass {
	public:
		rcRudderMode (boatStateClass& state, rcModeEnum last = rcModeEnum::NONE) :
			rcModeBaseClass(state, last, rcModeEnum::RUDDER) {};
};

class rcCourseMode : public rcModeBaseClass {
	public:
		rcCourseMode (boatStateClass& state, rcModeEnum last = rcModeEnum::NONE) :
			rcModeBaseClass(state, last, rcModeEnum::COURSE) {};
};

#endif
