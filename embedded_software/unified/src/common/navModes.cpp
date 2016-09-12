/******************************************************************************
 * Hackerboat Beaglebone navigation modes module
 * navModes.cpp
 * This is the navigation sub-modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
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
#include "navModes.hpp"

NavModeBase *NavModeBase::factory(BoatState& state, NavModeEnum mode) {
	switch (mode) {
		case NavModeEnum::IDLE:
			return new NavIdleMode(state, state.getNavMode());
			break;
		case NavModeEnum::FAULT:
			return new NavFaultMode(state, state.getNavMode());
			break;
		case NavModeEnum::RC:
			return new NavRCMode(state, state.getNavMode());
			break;
		case NavModeEnum::AUTONOMOUS:
			return new NavAutoMode(state, state.getNavMode());
		case NavModeEnum::NONE:
		default:
			return new NavIdleMode(state, state.getBoatMode());
			break;
	}
}

NavModeBase *NavIdleMode::execute () {
	state.throttle->setThrottle(0);
	state.rudder->write(0);
	return this;
}

NavModeBase *NavFaultMode::execute () {
	
}

NavModeBase *NavRCMode::execute () {
	
}

NavModeBase *NavAutoMode::execute () {
	
}