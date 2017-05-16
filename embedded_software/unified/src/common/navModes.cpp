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
#include "easylogging++.h"
#include "util.hpp"
#include "configuration.hpp"

NavModeBase *NavModeBase::factory(BoatState& state, NavModeEnum mode) {
	switch (mode) {
		case NavModeEnum::IDLE:
			LOG(DEBUG) << "Creating new Idle nav mode object";
			return new NavIdleMode(state, state.getNavMode());
			break;
		case NavModeEnum::FAULT:
			LOG(DEBUG) << "Creating new Fault nav mode object";
			return new NavFaultMode(state, state.getNavMode());
			break;
		case NavModeEnum::RC:
			LOG(DEBUG) << "Creating new RC nav mode object";
			return new NavRCMode(state, state.getNavMode());
			break;
		case NavModeEnum::AUTONOMOUS:
			LOG(DEBUG) << "Creating new Autonomous nav mode object";
			return new NavAutoMode(state, state.getNavMode());
			break;
		case NavModeEnum::NONE:
		default:
			LOG(DEBUG) << "Creating new Idle nav mode object by default";
			return new NavIdleMode(state, state.getNavMode());
			break;
	}
}

NavModeBase *NavIdleMode::execute () {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting nav idle mode";
	}
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.throttle->setThrottle(0)) ? _state.insertFault("Throttle Fault") : _state.removeFault("Throttle Fault");
	(!_state.rudder->write(0)) ? _state.insertFault("Rudder Fault") : _state.removeFault("Rudder Fault");
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) {
		LOG(ERROR) << "Fault detected in nav idle mode: " << _state.getFaultString();
		return NavModeBase::factory(_state, NavModeEnum::FAULT);
	}
	
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	
	// read the next command
	NavModeEnum newmode = _state.getNavMode();
	if (newmode != NavModeEnum::IDLE) {
		LOG(DEBUG) << "Leaving nav idle mode for " << _state.navModeNames.get(_state.getNavMode()) << " by command";
		return NavModeBase::factory(_state, newmode);
	}
	
	// check for RC mode switch
	if (_state.rc->getChannel(Conf::get()->RCchannelMap().at("auto")) > Conf::get()->RClimits().at("middlePosn")) {	// Note that this will trip in the case of failsafe mode
		LOG(DEBUG) << "Leaving nav idle mode for RC mode by switch";
		return NavModeBase::factory(_state, NavModeEnum::RC);
	}
	
	return this;
}

NavModeBase *NavFaultMode::execute () {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting nav fault mode";
	}
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.throttle->setThrottle(0)) ? _state.insertFault("Throttle Fault") : _state.removeFault("Throttle Fault");
	(!_state.rudder->write(0)) ? _state.insertFault("Rudder Fault") : _state.removeFault("Rudder Fault");
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (!_state.faultCount()) {
		LOG(INFO) << "Nav mode faults resolved";
		return NavModeBase::factory(_state, NavModeEnum::IDLE);
	}
	
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	
	return this;
}

NavModeBase *NavRCMode::execute () {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting nav RC mode";
	}
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) {
		LOG(ERROR) << "Fault detected in nav RC mode: " << _state.getFaultString();
		return NavModeBase::factory(_state, NavModeEnum::FAULT);
	}
	
	// read the next command
	NavModeEnum newmode = _state.getNavMode();
	if (newmode != NavModeEnum::RC) {
		LOG(DEBUG) << "Leaving nav RC mode for " << _state.navModeNames.get(_state.getNavMode()) << " by command";
		return NavModeBase::factory(_state, newmode);
	}
	_state.setNavMode(NavModeEnum::RC);	// Overwrites any improper commands
	
	// check the auto/rc mode switch (nav mode commands will be ignored)
	if (_state.rc->getChannel(Conf::get()->RCchannelMap().at("auto")) < Conf::get()->RClimits().at("middlePosn")) {	// Note that this will trip in the case of failsafe mode
		LOG(DEBUG) << "Leaving nav RC mode for auto mode by switch";
		return NavModeBase::factory(_state, NavModeEnum::AUTONOMOUS);
	}
	_state.setNavMode(NavModeEnum::RC);	// Overwrites any improper commands
	
	// execute the current RC mode
	_oldRCmode = _rcMode;
	_rcMode = _rcMode->execute();
	if (_rcMode != _oldRCmode) REMOVE(_oldRCmode);
	
	return this;
}

NavModeBase *NavAutoMode::execute () {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting nav auto mode";
	}
	this->callCount++;
	_state.recordTime = std::chrono::system_clock::now();		// Get a consistent time for everything in this invocation
	(!_state.servoEnable.set()) ? _state.insertFault("Servo Enable Fault") : _state.removeFault("Servo Enable Fault");
	(!_state.adc->isValid()) ? _state.insertFault("ADC Invalid") : _state.removeFault("ADC Invalid");
	(!_state.orient->isValid()) ? _state.insertFault("IMU Invalid") : _state.removeFault("IMU Invalid");
	(!_state.gps->isValid()) ? _state.insertFault("GPS Invalid") : _state.removeFault("GPS Invalid");
	if (_state.faultCount()) {
		LOG(ERROR) << "Fault detected in nav auto mode: " << _state.getFaultString();
		return NavModeBase::factory(_state, NavModeEnum::FAULT);
	}
	
	// read the next command
	NavModeEnum newmode = _state.getNavMode();
	if (newmode != NavModeEnum::AUTONOMOUS) {
		LOG(DEBUG) << "Leaving nav auto mode for " << _state.navModeNames.get(_state.getNavMode()) << " by command";
		return NavModeBase::factory(_state, newmode);
	}
	_state.setNavMode(NavModeEnum::AUTONOMOUS);	// Overwrites any improper commands
	
	// check for RC mode switch
	if (_state.rc->getChannel(Conf::get()->RCchannelMap().at("auto")) > Conf::get()->RClimits().at("middlePosn")) {	// Note that this will trip in the case of failsafe mode
		LOG(DEBUG) << "Leaving nav auto mode for RC mode by switch";
		return NavModeBase::factory(_state, NavModeEnum::RC);
	}
	
	// execute the current RC mode
	_oldAutoMode = _autoMode;
	_autoMode = _autoMode->execute();
	if (_autoMode != _oldAutoMode) REMOVE(_oldAutoMode);
	
	return this;
}
