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

class AutoModeBase : public StateMachineBase<AutoModeEnum, BoatState> {
	public:
		static AutoModeBase* factory(BoatState& state, AutoModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~AutoModeBase () {};
	protected:
		AutoModeBase (BoatState& state, AutoModeEnum last, AutoModeEnum thisMode) :
			StateMachineBase<AutoModeEnum, BoatState> (state, last, thisMode) {};
};

class AutoIdleMode : public AutoModeBase {
	public:
		AutoIdleMode (BoatState& state, AutoModeEnum last = AutoModeEnum::NONE) : 
			AutoModeBase(state, last, AutoModeEnum::IDLE) {};
};

class AutoWaypointMode : public AutoModeBase {
	public:
		AutoWaypointMode (BoatState& state, AutoModeEnum last = AutoModeEnum::NONE) : 
			AutoModeBase(state, last, AutoModeEnum::WAYPOINT) {}; 
};

class AutoReturnMode : public AutoModeBase {
	public:
		AutoReturnMode (BoatState& state, AutoModeEnum last = AutoModeEnum::NONE) : 
			AutoModeBase(state, last, AutoModeEnum::RETURN) {}; 
};

class AutoAnchorMode : public AutoModeBase {
	public:
		AutoAnchorMode (BoatState& state, AutoModeEnum last = AutoModeEnum::NONE) : 
			AutoModeBase(state, last, AutoModeEnum::ANCHOR) {}; 
};

#endif