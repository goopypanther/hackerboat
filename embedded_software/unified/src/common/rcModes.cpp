/******************************************************************************
 * Hackerboat Beaglebone RC modes module
 * rcModes.cpp
 * This is the RC sub-modes
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
#include <tuple>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"
#include "pid.hpp"
#include "rcModes.hpp"
#include "easylogging++.h"

RCModeBase *RCModeBase::factory(BoatState& state, RCModeEnum mode) {
	switch(mode) {
		case RCModeEnum::IDLE:
			LOG(INFO) << "Creating new Idle RC mode object";
			return new RCIdleMode(state, state.getRCMode());
			break;
		case RCModeEnum::RUDDER:
			LOG(INFO) << "Creating new Rudder RC mode object";
			return new RCRudderMode(state, state.getRCMode());
			break;
		case RCModeEnum::COURSE:
			LOG(INFO) << "Creating new Course RC mode object";
			return new RCCourseMode(state, state.getRCMode());
			break;
		case RCModeEnum::FAILSAFE:
			LOG(INFO) << "Creating new Failsafe RC mode object";
			return new RCFailsafeMode(state, state.getRCMode());
			break;
		default:
			return new RCIdleMode(state, state.getRCMode());
			break;
	}
}

RCModeBase *RCIdleMode::execute() {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting RC idle mode";
	}
	callCount++;
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::IDLE) {
		LOG(INFO) << "Switching to RC mode " << _state.rcModeNames.get(_state.rc->getMode()) << " by switch";
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}

RCModeBase *RCRudderMode::execute() {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting RC rudder mode";
	}
	callCount++;
	// Write the outgoing rudder command
	_state.rudder->write(_state.rc->getRudder());
	LOG_EVERY_N(100, INFO) << "Rudder command: " << to_string(_state.rc->getRudder());
	// Set the throttle
	_state.throttle->setThrottle(_state.rc->getThrottle());
	LOG_EVERY_N(100, INFO) << "Throttle command: " << to_string(_state.rc->getThrottle());
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::RUDDER) {
		LOG(INFO) << "Switching to RC mode " << _state.rcModeNames.get(_state.rc->getMode()) << " by switch";
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}

RCModeBase *RCCourseMode::execute() {
	if ((helm.GetKp() != std::get<0>(_state.K)) ||
		(helm.GetKi() != std::get<1>(_state.K)) ||
		(helm.GetKd() != std::get<2>(_state.K))) {
			helm.SetTunings(std::get<0>(_state.K), 
							std::get<1>(_state.K), 
							std::get<2>(_state.K));
		}
	if (this->callCount == 0) {
		LOG(INFO) << "Starting RC course mode";
	}
	callCount++;
	// Grab the current orientation and find the heading error for the PID loop
	in = _state.orient->getOrientation()->headingError(_state.rc->getCourse());
	// Execute the PID process
	LOG_EVERY_N(100, INFO) << "True Heading: " << _state.orient->getOrientation()->makeTrue() 
							<< ", Target Course: " << _state.rc->getCourse();
	helm.Compute();	
	// Write the outgoing rudder command
	_state.rudder->write(out);
	LOG_EVERY_N(100, INFO) << "Rudder command: " << to_string(this->out);
	// Set the throttle
	_state.throttle->setThrottle(_state.rc->getThrottle());
	LOG_EVERY_N(100, INFO) << "Throttle command: " << to_string(_state.rc->getThrottle());
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::COURSE) {
		LOG(INFO) << "Switching to RC mode " << _state.rcModeNames.get(_state.rc->getMode()) << " by switch";
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}

RCModeBase *RCFailsafeMode::execute() {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting RC failsafe mode";
	}
	callCount++;
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::FAILSAFE) {
		LOG(INFO) << "Switching to RC mode " << _state.rcModeNames.get(_state.rc->getMode()) << " by switch";
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}