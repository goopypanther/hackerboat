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
#include "json_utilities.hpp"

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
	state.getLastRecord();	// grab the last record, so we can go into that state if necessary. 
	return BoatModeBase::factory(state, BoatModeEnum::SELFTEST);
}

BoatModeBase* BoatSelfTestMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up...
		oldState = state;					// Copy the old state so we can exit into the correct mode. 
		state.clearFaults();
	}	
	this->callCount++;
	servoEnable.set();
	
	// check for errors
	if (state.health->batteryMon < SYSTEM_START_BATTERY_MIN) state.insertFault("Low Battery");
	if (!state.health->isValid()) state.insertFault("Invalid Health Monitor");
	if (!state.rc->isValid()) state.insertFault("RC input invalid");
	if (!state.adc->isValid()) state.insertFault("ADC input invalid");
	if (!state.gps->isValid()) state.insertFault("GPS input invalid");
	if (!state.lastFix->isValid()) state.insertFault("Last GPS fix invalid");
	if (state.disarmInput.get() < 0) state.insertFault("Disarm input invalid");
	if (state.armInput.get() < 0) state.insertFault("Arm input invalid");
	if (state.disarmInput.get() == state.armInput.get()) state.insertFault("Arm/disarm inputs do not agree");
	for (auto const r : *(state.relays->getmap())) {
		if (!r.second.isInitialized()) state.insertFault("Relay " + r.first +  " did not initialize");
		if (r.second.isFaulted()) state.insertFault("Relay " + r.first + " is faulted")
	}
	
	// Are we done?
	if (recordTime > (start + SELFTEST_DELAY) {
		if (state.faultCount()) {
			return BoatModeBase::factory(state, BoatModeEnum::FAULT);
		} else {
			if ((oldState.boat() = BoatModeEnum::NAVIGATION) && (state.armInput.get() > 0)) {
				return new BoatNavigationMode(state, state.getBoatMode(), oldState.getNavMode());
			} else {
				state.relays->get("DISARM").set();
				std::this_thread::sleep_for(DISARM_PULSE_LEN);
				state.relays->get("DISARM").clear();
				servoEnable.clear();
				return BoatModeBase::factory(state, BoatModeEnum::DISARMED);
			}
		}
	}
	return this;
}

BoatModeBase* BoatDisarmedMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	this->callCount++;
	
	state.servoEnable.clear();
	state.throttle->setThrottle(0);
	
	// read the next command
	if (state.cmdvec.size()) {
		if (state.cmdvec.front().getCmd() == "SetMode") {
			BoatModeEnum newmode;
			std::string newmodename;
			::parse(json_object_get(input, "mode"), &newmodename);
			if (boatModeNames.get(newmodename, &newmode)) {
				state.cmdvec.erase(state.cmdvec.begin());
				if (newmode == BoatModeEnum::SELFTEST) {
					return BoatModeBase::factory(state, newmode);
				} else if (newmode == BoatModeEnum::NAVIGATION) {
					state.relays->get("ENABLE").set();
					std::this_thread::sleep_for(ARM_PULSE_LEN);
					state.relays->get("ENABLE").clear();
					return BoatModeBase::factory(state, newmode);
				}
			}
		} else {
			state.cmdvec.front().execute();
		}
		state.cmdvec.erase(state.cmdvec.begin());
	}
	
	// check if the boat has been armed
	if (state.armInput.get() > 0) {
		if (hornOn) {
			if (recordTime > (hornStartTime + HORN_TIME)) {
				state.relays->get("HORN").clear();
				return BoatModeBase::factory(state, BoatModeEnum::NAVIGATION);
			}
		} else {
			hornOn = true;
			hornStartTime = sysclock::now();
			state.relays->get("HORN").set();
		}
	}
	
	// check if we've got a fault
	if (state.faultCount()) {
		return BoatModeBase::factory(state, BoatModeEnum::FAULT);
	}	
	return this;
}

BoatModeBase* BoatFaultMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	if (!callCount) {						// Execute only on the first invocation... 
		state.relays->get("DISARM").set();
		std::this_thread::sleep_for(DISARM_PULSE_LEN);
		state.relays->get("DISARM").clear();
	}
	this->callCount++;
	state.throttle->setThrottle(0);
	
	// read the command stack
	if (state.cmdvec.size()) {
		if (state.cmdvec.front().getCmd() == "SetMode") {
			BoatModeEnum newmode;
			std::string newmodename;
			::parse(json_object_get(input, "mode"), &newmodename);
			if (boatModeNames.get(newmodename, &newmode)) {
				if (newmode == BoatModeEnum::SELFTEST) {
					state.cmdvec.erase(state.cmdvec.begin());
					return BoatModeBase::factory(state, newmode);
				}
			}
		} else {
			state.cmdvec.front().execute();
		}
		state.cmdvec.erase(state.cmdvec.begin());
	}
	
	// check if the error has cleared
	if (state.health->batteryMon > SYSTEM_START_BATTERY_MIN) state.removeFault("Low Battery");
	if (state.health->isValid()) state.removeFault("Invalid Health Monitor");
	if (state.rc->isValid()) state.removeFault("RC input invalid");
	if (state.adc->isValid()) state.removeFault("ADC input invalid");
	if (state.gps->isValid()) state.removeFault("GPS input invalid");
	if (state.lastFix->isValid()) state.removeFault("Last GPS fix invalid");
	if (state.disarmInput.get() > 0) state.removeFault("Disarm input invalid");
	if (state.armInput.get() > 0) state.removeFault("Arm input invalid");
	if (state.disarmInput.get() != state.armInput.get()) state.removeFault("Arm/disarm inputs do not agree");
	for (auto const r : *(state.relays->getmap())) {
		if (r.second.isInitialized()) state.removeFault("Relay " + r.first +  " did not initialize");
		if (!r.second.isFaulted()) state.removeFault("Relay " + r.first + " is faulted")
	}

	// check if we've cleared our error
	if (state.faultCount()) {
		return this;
	} else return BoatModeBase::factory(state, this->getLastMode());
}

BoatModeBase* BoatNavigationMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	this->callCount++;
	
	// read the next command
	if (state.cmdvec.size()) {
		if (state.cmdvec.front().getCmd() == "SetMode") {
			BoatModeEnum newmode;
			std::string newmodename;
			::parse(json_object_get(input, "mode"), &newmodename);
			if (boatModeNames.get(newmodename, &newmode)) {
				state.cmdvec.erase(state.cmdvec.begin());
				if (newmode == BoatModeEnum::SELFTEST) {
					return BoatModeBase::factory(state, newmode);
				} else if (newmode == BoatModeEnum::DISARM) {
					state.relays->get("DISARM").set();
					std::this_thread::sleep_for(DISARM_PULSE_LEN);
					state.relays->get("DISARM").clear();
					return BoatModeBase::factory(state, newmode);
				}
			}
		} else {
			state.cmdvec.front().execute();
		}
		state.cmdvec.erase(state.cmdvec.begin());
	}
	
	// execute the current nav state
	_oldNavMode = _navMode;
	_navMode = _navMode->execute();
	if (_navMode != _oldNavMode) delete _oldNavMode;

	// check if we have a fault or a disarm signal
	if (state.faultCount() || (state._navMode == NavModeEnum::FAULT)) {
		state.setNavMode(NavModeEnum::FAULT);
		delete _navMode;
		_navMode = NavModeBase::factory(state, NavModeEnum::FAULT);
		return BoatModeBase::factory(state, BoatModeEnum::FAULT);
	}
	if ((state.disarmInput.get() == 1) || (state.armInput() != 1)) {
		return BoatModeBase::factory(state, BoatModeEnum::DISARMED)
	}
	
	return this;
}

BoatModeBase* BoatArmedTestMode::execute() {
	this->callCount++;
	return BoatModeBase::factory(state, BoatModeEnum::DISARMED);
}