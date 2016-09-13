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
#include "hackerboatRoot.hpp"
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
	_state.getLastRecord();	// grab the last record, so we can go into that state if necessary. 
	return BoatModeBase::factory(_state, BoatModeEnum::SELFTEST);
}

BoatModeBase* BoatSelfTestMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up...
		oldState = _state;					// Copy the old state so we can exit into the correct mode. 
		_state.clearFaults();
	}	
	this->callCount++;
	_state.servoEnable.set();
	
	// check for errors
	if (_state.health->batteryMon < SYSTEM_START_BATTERY_MIN) _state.insertFault("Low Battery");
	if (!_state.health->isValid()) _state.insertFault("Invalid Health Monitor");
	if (!_state.rc->isValid()) _state.insertFault("RC input invalid");
	if (!_state.adc->isValid()) _state.insertFault("ADC input invalid");
	if (!_state.gps->isValid()) _state.insertFault("GPS input invalid");
	if (!_state.lastFix.isValid()) _state.insertFault("Last GPS fix invalid");
	if (_state.disarmInput.get() < 0) _state.insertFault("Disarm input invalid");
	if (_state.armInput.get() < 0) _state.insertFault("Arm input invalid");
	if (_state.disarmInput.get() == _state.armInput.get()) _state.insertFault("Arm/disarm inputs do not agree");
	for (auto r : *(_state.relays->getmap())) {
		if (!r.second.isInitialized()) _state.insertFault("Relay " + r.first +  " did not initialize");
		if (r.second.isFaulted()) _state.insertFault("Relay " + r.first + " is faulted");
	}
	
	// Are we done?
	if (_state.recordTime > (start + SELFTEST_DELAY)) {
		if (_state.faultCount()) {
			return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
		} else {
			if ((oldState.getBoatMode() == BoatModeEnum::NAVIGATION) && (_state.armInput.get() > 0)) {
				return new BoatNavigationMode(_state, _state.getBoatMode(), oldState.getNavMode());
			} else {
				_state.relays->get("DISARM").set();
				std::this_thread::sleep_for(DISARM_PULSE_LEN);
				_state.relays->get("DISARM").clear();
				_state.servoEnable.clear();
				return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
			}
		}
	}
	return this;
}

BoatModeBase* BoatDisarmedMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	this->callCount++;
	
	_state.servoEnable.clear();
	_state.throttle->setThrottle(0);
	
	// check if a command has changed our state
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::DISARMED) {
		if (newmode == BoatModeEnum::SELFTEST) {
			return BoatModeBase::factory(_state, newmode);
		} else if (newmode == BoatModeEnum::NAVIGATION) {
			_state.relays->get("ENABLE").set();
			std::this_thread::sleep_for(ARM_PULSE_LEN);
			_state.relays->get("ENABLE").clear();
			return BoatModeBase::factory(_state, newmode);
		}
	} else _state.setBoatMode(BoatModeEnum::DISARMED);
	
	// check if the boat has been armed
	if (_state.armInput.get() > 0) {
		if (hornOn) {
			if (_state.recordTime > (hornStartTime + HORN_TIME)) {
				_state.relays->get("HORN").clear();
				return BoatModeBase::factory(_state, BoatModeEnum::NAVIGATION);
			}
		} else {
			hornOn = true;
			hornStartTime = std::chrono::system_clock::now();
			_state.relays->get("HORN").set();
		}
	}
	
	// check if we've got a fault
	if (_state.faultCount()) {
		return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
	}	
	return this;
}

BoatModeBase* BoatFaultMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	if (!callCount) {						// Execute only on the first invocation... 
		_state.relays->get("DISARM").set();
		std::this_thread::sleep_for(DISARM_PULSE_LEN);
		_state.relays->get("DISARM").clear();
	}
	this->callCount++;
	_state.throttle->setThrottle(0);
	
	// read the command stack
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::FAULT) {
		if (newmode == BoatModeEnum::SELFTEST) {
			return BoatModeBase::factory(_state, newmode);
		} 
	} else _state.setBoatMode(BoatModeEnum::FAULT);
	
	// check if the error has cleared
	if (_state.health->batteryMon > SYSTEM_START_BATTERY_MIN) _state.removeFault("Low Battery");
	if (_state.health->isValid()) _state.removeFault("Invalid Health Monitor");
	if (_state.rc->isValid()) _state.removeFault("RC input invalid");
	if (_state.adc->isValid()) _state.removeFault("ADC input invalid");
	if (_state.gps->isValid()) _state.removeFault("GPS input invalid");
	if (_state.lastFix.isValid()) _state.removeFault("Last GPS fix invalid");
	if (_state.disarmInput.get() > 0) _state.removeFault("Disarm input invalid");
	if (_state.armInput.get() > 0) _state.removeFault("Arm input invalid");
	if (_state.disarmInput.get() != _state.armInput.get()) _state.removeFault("Arm/disarm inputs do not agree");
	for (auto r : *(_state.relays->getmap())) {
		if (r.second.isInitialized()) _state.removeFault("Relay " + r.first +  " did not initialize");
		if (!r.second.isFaulted()) _state.removeFault("Relay " + r.first + " is faulted");
	}

	// check if we've cleared our error
	if (_state.faultCount()) {
		return this;
	} else return BoatModeBase::factory(_state, this->getLastMode());
}

BoatModeBase* BoatNavigationMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	this->callCount++;
	
	// read the next command
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::NAVIGATION) {
		if (newmode == BoatModeEnum::SELFTEST) {
			return BoatModeBase::factory(_state, newmode);
		} else if (newmode == BoatModeEnum::DISARMED) {
			_state.relays->get("DISARM").set();
			std::this_thread::sleep_for(DISARM_PULSE_LEN);
			_state.relays->get("DISARM").clear();
			return BoatModeBase::factory(_state, newmode);
		}
	} else _state.setBoatMode(BoatModeEnum::NAVIGATION);
	
	// execute the current nav state
	_oldNavMode = _navMode;
	_navMode = _navMode->execute();
	if (_navMode != _oldNavMode) delete _oldNavMode;

	// check if we have a fault or a disarm signal
	if (_state.faultCount() || (_state.getNavMode() == NavModeEnum::FAULT)) {
		_state.setNavMode(NavModeEnum::FAULT);
		delete _navMode;
		_navMode = NavModeBase::factory(_state, NavModeEnum::FAULT);
		return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
	}
	if ((_state.disarmInput.get() == 1) || (_state.armInput.get() != 1)) {
		return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
	}
	
	return this;
}

BoatModeBase* BoatArmedTestMode::execute() {
	this->callCount++;
	return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
}