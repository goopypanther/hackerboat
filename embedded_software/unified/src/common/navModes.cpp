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
			return new NavIdleMode(state, state.getNavMode());
			break;
	}
}

NavModeBase *NavIdleMode::execute () {
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.throttle->setThrottle(0)) ? _state.insertFault("Throttle Fault") : _state.removeFault("Throttle Fault");
	(!_state.rudder->write(0)) ? _state.insertFault("Rudder Fault") : _state.removeFault("Rudder Fault");
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) return NavModeBase::factory(_state, NavModeEnum::FAULT);
	
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	
	// read the next command
	NavModeEnum newmode = _state.getNavMode();
	if (newmode != NavModeEnum::IDLE) {
		return NavModeBase::factory(_state, newmode);
	}
	
	// check for RC mode switch
	if (_state.rc->getChannel(RC_AUTO_SWITCH) > RC_MIDDLE_POSN) {	// Note that this will trip in the case of failsafe mode
		return NavModeBase::factory(_state, NavModeEnum::RC);
	}
	
	return this;
}

NavModeBase *NavFaultMode::execute () {
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.throttle->setThrottle(0)) ? _state.insertFault("Throttle Fault") : _state.removeFault("Throttle Fault");
	(!_state.rudder->write(0)) ? _state.insertFault("Rudder Fault") : _state.removeFault("Rudder Fault");
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (!_state.faultCount()) return NavModeBase::factory(_state, NavModeEnum::IDLE);
	
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	
	return this;
}

NavModeBase *NavRCMode::execute () {
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) return NavModeBase::factory(_state, NavModeEnum::FAULT);
	
	// check the auto/rc mode switch (nav mode commands will be ignored)
	if (_state.rc->getChannel(RC_AUTO_SWITCH) < RC_MIDDLE_POSN) {	// Note that this will trip in the case of failsafe mode
		return NavModeBase::factory(_state, NavModeEnum::AUTONOMOUS);
	}
	_state.setNavMode(NavModeEnum::RC);	// Overwrites any improper commands
	
	// execute the current RC mode
	_oldRCmode = _rcMode;
	_rcMode = _rcMode->execute();
	if (_rcMode != _oldRCmode) delete _oldRCmode;
	
	return this;
}

NavModeBase *NavAutoMode::execute () {
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) return NavModeBase::factory(_state, NavModeEnum::FAULT);
	
	// read the next command
	NavModeEnum newmode = _state.getNavMode();
	if (newmode != NavModeEnum::AUTONOMOUS) {
		return NavModeBase::factory(_state, newmode);
	}
	
	// check for RC mode switch
	if (_state.rc->getChannel(RC_AUTO_SWITCH) > RC_MIDDLE_POSN) {	// Note that this will trip in the case of failsafe mode
		return NavModeBase::factory(_state, NavModeEnum::RC);
	}
	
	// execute the current RC mode
	_oldAutoMode = _autoMode;
	_autoMode = _autoMode->execute();
	if (_autoMode != _oldAutoMode) delete _oldAutoMode;
	
	return this;
}