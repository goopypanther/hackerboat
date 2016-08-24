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

class NavModeBase : public StateMachineBase<NavModeEnum, BoatState> {
	public:
		static NavModeBase* factory(BoatState& state, NavModeEnum mode);	/**< Create a new object of the given mode */
		virtual ~NavModeBase() {};
	protected:
		NavModeBase (BoatState& state, NavModeEnum last, NavModeEnum thisMode) :
			StateMachineBase<NavModeEnum, BoatState> (state, last, thisMode) {};
		
};

class NavIdleMode : public NavModeBase {
	public:
		NavIdleMode (BoatState& state, NavModeEnum last = NavModeEnum::NONE) : 
			NavModeBase(state, last, NavModeEnum::IDLE) {};
};

class NavFaultMode : public NavModeBase {
	public:
		NavFaultMode (BoatState& state, NavModeEnum last = NavModeEnum::NONE) : 
			NavModeBase(state, last, NavModeEnum::FAULT) {};
};

class NavRCMode : public NavModeBase {
	public:
		NavRCMode (BoatState& state, NavModeEnum last = NavModeEnum::NONE, RCModeEnum submode = RCModeEnum::IDLE) : 
			NavModeBase(state, last, NavModeEnum::RC),
			_rcMode(RCModeBase::factory(state, submode)) {};
		RCModeBase* getRCMode () {return _rcMode;};				/**< Get the current RC mode object */
		~NavRCMode () {delete _rcMode;};
	private:
		RCModeBase* _rcMode;
};

class NavAutoMode : public NavModeBase {
	public:
		NavAutoMode (BoatState& state, NavModeEnum last = NavModeEnum::NONE, AutoModeEnum submode = AutoModeEnum::IDLE) : 
			NavModeBase(state, last, NavModeEnum::AUTONOMOUS),
			_autoMode(AutoModeBase::factory(state, submode)) {};
		AutoModeBase* getAutoMode () {return _autoMode;};			/**< Get the current autonomous mode object */
		~NavAutoMode () {delete _autoMode;};
	private:
		AutoModeBase* _autoMode;
};

#endif
