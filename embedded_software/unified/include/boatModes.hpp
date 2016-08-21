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
	protected:
		boatModeBaseClass (boatStateClass& state, boatModeEnum last, boatModeEnum thisMode) :
			stateMachineBaseClass<boatModeEnum, boatStateClass> (state, last, thisMode) {};
};

class boatStartMode : public boatModeBaseClass {
	public:
		boatStartMode  (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE) : 
			boatModeBaseClass(state, boatModeEnum::NONE, boatModeEnum::START) {}; 
};

class boatSelfTestMode : public boatModeBaseClass {
	public:
		boatSelfTestMode (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE) : 
			boatModeBaseClass(state, last, boatModeEnum::SELFTEST) {};  
};

class boatDisarmedMode : public boatModeBaseClass {
	public:
		boatDisarmedMode (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE) : 
			boatModeBaseClass(state, last, boatModeEnum::DISARMED) {}; 
};

class boatFaultMode : public boatModeBaseClass {
	public:
		boatFaultMode (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE) : 
			boatModeBaseClass(state, last, boatModeEnum::FAULT) {};
};

class boatNavigationMode : public boatModeBaseClass {
	public:
		boatNavigationMode (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE, navigationModeEnum submode = navigationModeEnum::IDLE) : 
			boatModeBaseClass(state, last, boatModeEnum::NAVIGATION),
			_navMode(navModeBaseClass::navModeFactory(state, submode)) {};
		navModeBaseClass* getNavMode () {return _navMode;};		/**< Get the current nav mode object */
		~boatNavigationMode () {delete _navMode;};
	private:
		navModeBaseClass* _navMode;
};

class boatArmedTestMode : public boatModeBaseClass {
	public:
		boatArmedTestMode (boatStateClass& state, boatModeEnum last = boatModeEnum::NONE) : 
			boatModeBaseClass(state, last, boatModeEnum::ARMEDTEST) {};
};

#endif