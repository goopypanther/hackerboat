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
#include "rcModes.hpp"
#include "autoModes.hpp"

class navModeBaseClass : public stateMachineBaseClass<navigationModeEnum, boatStateClass> {
	public:
		static navModeBaseClass* navModeFactory(boatStateClass& state, navigationModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~navModeBaseClass() {};
	protected:
		navModeBaseClass (boatStateClass& state, navigationModeEnum last, navigationModeEnum thisMode) :
			stateMachineBaseClass<navigationModeEnum, boatStateClass> (state, last, thisMode) {};
		
};

class navIdleMode : public navModeBaseClass {
	public:
		navIdleMode (boatStateClass& state, navigationModeEnum last = navigationModeEnum::NONE) : 
			navModeBaseClass(state, last, navigationModeEnum::IDLE) {};
};

class navFaultMode : public navModeBaseClass {
	public:
		navFaultMode (boatStateClass& state, navigationModeEnum last = navigationModeEnum::NONE) : 
			navModeBaseClass(state, last, navigationModeEnum::FAULT) {};
};

class navRCMode : public navModeBaseClass {
	public:
		navRCMode (boatStateClass& state, navigationModeEnum last = navigationModeEnum::NONE, rcModeEnum submode = rcModeEnum::IDLE) : 
			navModeBaseClass(state, last, navigationModeEnum::RC),
			_rcMode(rcModeBaseClass::rcModeFactory(state, submode)) {};
		rcModeBaseClass* getRCMode () {return _rcMode;};				/**< Get the current RC mode object */
		~navRCMode () {delete _rcMode;};
	private:
		rcModeBaseClass* _rcMode;
};

class navAutoMode : public navModeBaseClass {
	public:
		navAutoMode (boatStateClass& state, navigationModeEnum last = navigationModeEnum::NONE, autoModeEnum submode = autoModeEnum::IDLE) : 
			navModeBaseClass(state, last, navigationModeEnum::AUTONOMOUS),
			_autoMode(autoModeBaseClass::autoModeFactory(state, submode)) {};
		autoModeBaseClass* getAutoMode () {return _autoMode;};			/**< Get the current autonomous mode object */
		~navAutoMode () {delete _autoMode;};
	private:
		autoModeBaseClass* _autoMode;
};

#endif
