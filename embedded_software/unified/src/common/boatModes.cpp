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
#include "easylogging++.h"

BoatModeBase* BoatModeBase::factory(BoatState& state, BoatModeEnum mode) {
	switch (mode) {
		case BoatModeEnum::START:
			LOG(DEBUG) << "Creating new Start boat mode object";
			return new BoatStartMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::SELFTEST:
			LOG(DEBUG) << "Creating new Self Test boat mode object";
			return new BoatSelfTestMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::DISARMED:
			LOG(DEBUG) << "Creating new Disarmed boat mode object";
			return new BoatDisarmedMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::FAULT:
			LOG(DEBUG) << "Creating new Fault boat mode object";
			return new BoatFaultMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::NAVIGATION:
			LOG(DEBUG) << "Creating new Navigation boat mode object with submode " << state.navModeNames.get(state.getNavMode());
			return new BoatNavigationMode(state, state.getBoatMode(), state.getNavMode());
			break;
		case BoatModeEnum::ARMEDTEST:
			LOG(DEBUG) << "Creating new Armed Test boat mode object";
			return new BoatArmedTestMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::LOWBATTERY:
			LOG(DEBUG) << "Creating new Low Battery boat mode object";
			return new BoatLowBatteryMode(state, state.getBoatMode());
			break;
		case BoatModeEnum::NONE:
		default:
			LOG(DEBUG) << "Creating new Start boat mode object";
			return new BoatStartMode(state, state.getBoatMode());
			break;
	}
}

BoatModeBase* BoatStartMode::execute() {
	_state.getLastRecord();	// grab the last record, so we can go into that state if necessary. 
	LOG(INFO) << "Entering self test from start mode";
	return BoatModeBase::factory(_state, BoatModeEnum::SELFTEST);
}

BoatModeBase* BoatSelfTestMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up... 
		LOG(INFO) << "Starting self-test";
		_state.clearFaults();
		oldState = _state;					// Copy the old state so we can exit into the correct mode.
	}	
	this->callCount++;
	cout << "Self Test call count: " << callCount << endl;
	_state.servoEnable.set();
	
	// check for errors
	if (_state.health->batteryMon < SYSTEM_START_BATTERY_MIN) _state.insertFault("Low Battery");
	if (!_state.health->isValid()) _state.insertFault("Invalid Health Monitor");
	if (!_state.rc->isValid()) _state.insertFault("RC input invalid");
	if (!_state.adc->isValid()) _state.insertFault("ADC input invalid");
	if (!_state.gps->isValid()) _state.insertFault("GPS input invalid");
	if (!_state.lastFix.isValid()) _state.insertFault("Last GPS fix invalid");
	if (_state.getArmState() == ArmButtonStateEnum::INVALID) {
		_state.insertFault("Arm and/or Disarm input is invalid");
	}
	//for (auto r : *(_state.relays->getmap())) {
	//	if (!r.second.isInitialized()) _state.insertFault("Relay " + r.first +  " did not initialize");
	//	if (r.second.isFaulted()) _state.insertFault("Relay " + r.first + " is faulted");
	//}
	LOG_IF(_state.faultCount(), ERROR) << "Faults: [" << _state.getFaultString() << "]";
	
	// Check for things that might clear
	if (_state.lastFix.isValid()) _state.removeFault("Last GPS fix invalid");
	if (_state.health->batteryMon > SYSTEM_START_BATTERY_MIN) _state.removeFault("Low Battery");
	if (_state.rc->isValid()) _state.removeFault("RC input invalid");
	
	// Are we done?
	if (_state.recordTime > (start + SELFTEST_DELAY)) {
		if (_state.faultCount()) {
			if (_state.hasFault("Low Battery") && (_state.faultCount() == 1)) {
				LOG(INFO) << "Entering low battery mode from self test";
				return new BoatLowBatteryMode(_state, oldState.getBoatMode());
			}
			LOG(INFO) << "Entering fault mode from self test";
			return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
		} else {
			if ((oldState.getBoatMode() == BoatModeEnum::NAVIGATION) && (_state.getArmState() == ArmButtonStateEnum::ARM)) {
				LOG(INFO) << "Entering navigation mode from self test";
				return new BoatNavigationMode(_state, _state.getBoatMode(), oldState.getNavMode());
			} else {
				LOG(INFO) << "Entering disarm mode from self test";
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
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up... 
		LOG(INFO) << "Starting disarmed mode";
	}
	this->callCount++;
	cout << "Disarm call count: " << callCount << endl;
	
	_state.servoEnable.clear();
	_state.throttle->setThrottle(0);
	
	// check if a command has changed our state
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::DISARMED) {
		if (newmode == BoatModeEnum::SELFTEST) {
			return BoatModeBase::factory(_state, newmode);
		} else if (newmode == BoatModeEnum::NAVIGATION) {
			LOG(INFO) << "Arming on remote command";
			_state.relays->get("ENABLE").set();
			std::this_thread::sleep_for(ARM_PULSE_LEN);
			_state.relays->get("ENABLE").clear();
			_state.setBoatMode(BoatModeEnum::DISARMED);
		} else _state.setBoatMode(BoatModeEnum::DISARMED);
	} 
	
	// check if the boat has been armed
	if (_state.getArmState() == ArmButtonStateEnum::ARM) {
		if (hornOn) {
			if (_state.recordTime > (hornStartTime + HORN_TIME)) {
				_state.relays->get("HORN").clear();
				LOG(INFO) << "Boat armed; turning off horn and entering navigation mode";
				return BoatModeBase::factory(_state, BoatModeEnum::NAVIGATION);
			}
		} else {
			hornOn = true;
			LOG(INFO) << "Boat armed; turning on horn";
			hornStartTime = std::chrono::system_clock::now();
			_state.relays->get("HORN").set();
		}
	} else {
		hornOn = false;
		LOG(INFO) << "Boat arming canceled";
		_state.relays->get("HORN").clear();
	}
	
	// check if we've got a fault
	if (_state.faultCount()) {
		LOG(ERROR) << "Exiting disarmed mode with faults: [" << _state.getFaultString() << "]";
		return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
	}	
	return this;
}

BoatModeBase* BoatFaultMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	if (!callCount) {						// Execute only on the first invocation... 
		LOG(ERROR) << "Entering fault mode with faults: [" << _state.getFaultString() << "]";
		_state.relays->get("DISARM").set();
		std::this_thread::sleep_for(DISARM_PULSE_LEN);
		_state.relays->get("DISARM").clear();
	}
	this->callCount++;
	cout << "Fault call count: " << callCount << endl;
	_state.throttle->setThrottle(0);
	_state.servoEnable.clear();
	
	// read the command stack
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::FAULT) {
		if (newmode == BoatModeEnum::SELFTEST) {
			LOG(INFO) << "Entering self test mode";
			return BoatModeBase::factory(_state, newmode);
		} else _state.setBoatMode(BoatModeEnum::FAULT);
	} 
	
	// check if the error has cleared
	if (_state.health->batteryMon > SYSTEM_START_BATTERY_MIN) _state.removeFault("Low Battery");
	if (_state.health->isValid()) _state.removeFault("Invalid Health Monitor");
	if (_state.rc->isValid()) _state.removeFault("RC input invalid");
	if (_state.adc->isValid()) _state.removeFault("ADC input invalid");
	if (_state.gps->isValid()) _state.removeFault("GPS input invalid");
	if (_state.lastFix.isValid()) _state.removeFault("Last GPS fix invalid");
	if (_state.getArmState() != ArmButtonStateEnum::INVALID) {
		_state.removeFault("Arm and/or Disarm input is invalid");
	}
	//for (auto r : *(_state.relays->getmap())) {
	//	if (r.second.isInitialized()) _state.removeFault("Relay " + r.first +  " did not initialize");
	//	if (!r.second.isFaulted()) _state.removeFault("Relay " + r.first + " is faulted");
	//}

	// check if we've cleared our error
	if (_state.faultCount()) {
		return this;
	} else return BoatModeBase::factory(_state, this->getLastMode());
}

BoatModeBase* BoatNavigationMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up... 
		LOG(INFO) << "Starting Navigation mode with submode " << _state.navModeNames.get(_state.getNavMode());
	}
	this->callCount++;
	
	//_state.servoEnable.set();
	
	// read the next command
	BoatModeEnum newmode = _state.getBoatMode();
	if (newmode != BoatModeEnum::NAVIGATION) {
		if (newmode == BoatModeEnum::SELFTEST) {
			LOG(INFO) << "Departing navigation mode for self test on shore command";
			delete _navMode;
			return new BoatSelfTestMode(_state, BoatModeEnum::NAVIGATION);
		} else if (newmode == BoatModeEnum::DISARMED) {
			LOG(INFO) << "Disarming on shore command";
			_state.relays->get("DISARM").set();
			std::this_thread::sleep_for(DISARM_PULSE_LEN);
			_state.relays->get("DISARM").clear();
			delete _navMode;
			return new BoatDisarmedMode(_state, BoatModeEnum::NAVIGATION);
		} else _state.setBoatMode(BoatModeEnum::NAVIGATION);
	} 
	
	// execute the current nav mode
	_oldNavMode = _navMode;
	_navMode = _navMode->execute();
	if (_navMode != _oldNavMode) delete _oldNavMode;

	// check the battery
	if (_state.health->batteryMon < SYSTEM_LOW_BATTERY_CUTOFF) {
		LOG(ERROR) << "Departing navigation mode in response to low battery";
		_state.insertFault("Low Battery");
		delete _navMode;
		return new BoatLowBatteryMode(_state, BoatModeEnum::NAVIGATION);
	}
	
	// check if we have a fault or a disarm signal
	if (_state.faultCount() || (_state.getNavMode() == NavModeEnum::FAULT)) {
		LOG(ERROR) << "Departing navigation mode in response to fault: [" << _state.getFaultString() << "]";
		_state.setNavMode(NavModeEnum::FAULT);
		delete _navMode;
		return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
	}
	if (_state.getArmState() == ArmButtonStateEnum::DISARM) {
		LOG(INFO) << "Exiting Navigation mode on disarm signal";
		delete _navMode;
		return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
	}
	
	return this;
}

BoatModeBase* BoatArmedTestMode::execute() {
	this->callCount++;
	LOG(WARNING) << "Armed Test mode is not implemented; disarming";
	return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
}

BoatModeBase* BoatLowBatteryMode::execute() {
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up... 
		LOG(INFO) << "Starting low battery mode with voltage " << _state.health->batteryMon << "V";
	}
	this->callCount++;
	
	LOG_EVERY_N(100, INFO) << "Battery voltage is " << _state.health->batteryMon << "V";
	_state.servoEnable.clear();
	_state.throttle->setThrottle(0);
	if (_state.health->batteryMon > SYSTEM_START_BATTERY_MIN) _state.removeFault("Low Battery");
	if (_state.faultCount() > 1) {
		return BoatModeBase::factory(_state, BoatModeEnum::FAULT);
	} else {
		if ((_lastMode == BoatModeEnum::NAVIGATION) && (_state.getArmState() == ArmButtonStateEnum::ARM)) {
			LOG(INFO) << "Recovering from low battery to navigation mode";
			return new BoatNavigationMode(_state, _lastMode, _state.getNavMode());
		} else {
			LOG(INFO) << "Recovering from low battery to disarm mode";
			_state.relays->get("DISARM").set();
			std::this_thread::sleep_for(DISARM_PULSE_LEN);
			_state.relays->get("DISARM").clear();
			_state.servoEnable.clear();
			return BoatModeBase::factory(_state, BoatModeEnum::DISARMED);
		}
	}
	return this;
}