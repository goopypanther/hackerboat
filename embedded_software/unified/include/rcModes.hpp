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

class RCModeBase : public StateMachineBase<RCModeEnum, BoatState> {
	public:
		static RCModeBase* factory(BoatState& state, RCModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~RCModeBase () {};
	protected:
		RCModeBase (BoatState& state, RCModeEnum last, RCModeEnum thisMode) :
			StateMachineBase<RCModeEnum, BoatState> (state, last, thisMode) {};
};

class RCIdleMode : public RCModeBase {
	public:
		RCIdleMode (BoatState& state, RCModeEnum last = RCModeEnum::NONE) :
			RCModeBase(state, last, RCModeEnum::IDLE) {};
};

class RCRudderMode : public RCModeBase {
	public:
		RCRudderMode (BoatState& state, RCModeEnum last = RCModeEnum::NONE) :
			RCModeBase(state, last, RCModeEnum::RUDDER) {};
};

class RCCourseMode : public RCModeBase {
	public:
		RCCourseMode (BoatState& state, RCModeEnum last = RCModeEnum::NONE) :
			RCModeBase(state, last, RCModeEnum::COURSE) {};
};

#endif
