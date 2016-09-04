/******************************************************************************
 * Hackerboat Beaglebone boat modes module
 * boatModes.cpp
 * This is the top-level boat modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
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
#include "navModes.hpp"
#include "boatModes.hpp"
#include "hal/relay.hpp"
#include "hal/gpio.hpp"

BoatModeBase* BoatModeBase::factory(BoatState& state, BoatModeEnum mode) {
	switch (mode) {
		case BoatModeEnum::START:
			return new BoatStartMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::SELFTEST:
			return new BoatSelfTestMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::DISARMED:
			return new BoatDisarmedMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::FAULT:
			return new BoatFaultMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::NAVIGATION:
			return new BoatNavigationMode(state, state.getBoatMode(), state.getNavMode());
			break;
		case BoatModeEnum::ARMEDTEST:
			return new BoatArmedTestMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::NONE:
		default:
			return new BoatStartMode(state, state.getBoatMode());
			break;
	}
}

BoatModeBase* BoatStartMode::execute() {
	this->callCount++;
	return BoatModeBase::factory(state, BoatModeEnum::SELFTEST);
}

BoatModeBase* BoatSelfTestMode::execute() {
	this->callCount++;
	sysclock present = sysclock::now();
	
	
	return this;
}

BoatModeBase* BoatDisarmedMode::execute() {
	this->callCount++;
	
}

BoatModeBase* BoatFaultMode::execute() {
	this->callCount++;
	
}

BoatModeBase* BoatNavigationMode::execute() {
	this->callCount++;
	
}

BoatModeBase* BoatArmedTestMode::execute() {
	this->callCount++;
	
}