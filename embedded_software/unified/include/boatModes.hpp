/******************************************************************************
 * Hackerboat Beaglebone boat modes module
 * boatModes.hpp
 * This is the top-level boat modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef BOATMODES_H
#define BOATMODES_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"
#include "navModes.hpp"

class BoatModeBase : public StateMachineBase<BoatModeEnum, BoatState> {
	protected:
		BoatModeBase (BoatState& state, BoatModeEnum last, BoatModeEnum thisMode) :
			StateMachineBase<BoatModeEnum, BoatState> (state, last, thisMode) {};
};

class BoatStartMode : public BoatModeBase {
	public:
		BoatStartMode  (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE) : 
			BoatModeBase(state, BoatModeEnum::NONE, BoatModeEnum::START) {}; 
};

class BoatSelfTestMode : public BoatModeBase {
	public:
		BoatSelfTestMode (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE) : 
			BoatModeBase(state, last, BoatModeEnum::SELFTEST) {};  
};

class BoatDisarmedMode : public BoatModeBase {
	public:
		BoatDisarmedMode (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE) : 
			BoatModeBase(state, last, BoatModeEnum::DISARMED) {}; 
};

class BoatFaultMode : public BoatModeBase {
	public:
		BoatFaultMode (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE) : 
			BoatModeBase(state, last, BoatModeEnum::FAULT) {};
};

class BoatNavigationMode : public BoatModeBase {
	public:
		BoatNavigationMode (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE, NavModeEnum submode = NavModeEnum::IDLE) : 
			BoatModeBase(state, last, BoatModeEnum::NAVIGATION),
			_navMode(NavModeBase::factory(state, submode)) {};
		NavModeBase* getNavMode () {return _navMode;};		/**< Get the current nav mode object */
		~BoatNavigationMode () {delete _navMode;};
	private:
		NavModeBase* _navMode;
};

class BoatArmedTestMode : public BoatModeBase {
	public:
		BoatArmedTestMode (BoatState& state, BoatModeEnum last = BoatModeEnum::NONE) : 
			BoatModeBase(state, last, BoatModeEnum::ARMEDTEST) {};
};

#endif