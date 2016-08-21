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

class boatModeBaseClass : public stateMachineBaseClass<boatModeEnum, boatStateClass> {
};

class boatStartMode : public boatModeBaseClass {
	public:
		boatStartMode  (boatStateClass& state, boatModeEnum last = NONE) : 
		_state(state), _thisMode(SELFTEST), _lastMode(last);
};

class boatSelfTestMode : public boatModeBaseClass {
	public:
		boatSelfTestMode (boatStateClass& state, boatModeEnum last = NONE) : 
			_state(state), _thisMode(SELFTEST), _lastMode(last);
};

class boatDisarmedMode : public boatModeBaseClass {
	public:
		boatDisarmedMode (boatStateClass& state, boatModeEnum last = NONE) : 
			_state(state), _thisMode(DISARMED), _lastMode(last);
};

class boatFaultMode : public boatModeBaseClass {
	public:
		boatFaultMode (boatStateClass& state, boatModeEnum last = NONE) : 
			_state(state), _thisMode(FAULT), _lastMode(last);
};

class boatNavigationMode : public boatModeBaseClass {
	public:
		boatNavigationMode (boatStateClass& state, navigationModeEnum = IDLE, boatModeEnum last = NONE) : 
			_state(state), _thisMode(NAVIGATION), _lastMode(last);
		navModeBaseClass& getNavMode () {return _navMode;};		/**< Get the current nav mode object */
	private:
		navModeBaseClass& _navMode;
};

class boatArmedTestMode : public boatModeBaseClass {
	public:
		boatArmedTestMode (boatStateClass& state, boatModeEnum last = NONE) : 
			_state(state), _thisMode(ARMEDTEST), _lastMode(last);
};

#endif