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
	state.getLastRecord();	// grab the last record, so we can go into that state if necessary. 
	return BoatModeBase::factory(state, BoatModeEnum::SELFTEST);
}

BoatModeBase* BoatSelfTestMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	
	if (this->callCount == 0) {				// Some housekeeping is in order if we just started up...
		oldState = state;					// Copy the old state so we can exit into the correct mode. 
	}	
	this->callCount++;
	servoEnable.set();
	
	if (recordTime > (start + SELFTEST_DELAY) {
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
	return this;
}

BoatModeBase* BoatFaultMode::execute() {
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	this->callCount++;
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
	if (state.faultCount()) {
		return this;
	} else return BoatModeBase::factory(state, this->getLastMode());
}

BoatModeBase* BoatNavigationMode::execute() {
	this->callCount++;
	this->recordTime = sysclock::now();		// Get a consistent time for everything in this invocation
	
}

BoatModeBase* BoatArmedTestMode::execute() {
	this->callCount++;
	
}