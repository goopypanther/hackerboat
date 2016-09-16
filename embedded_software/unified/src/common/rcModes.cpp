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

RCModeBase *RCModeBase::factory(BoatState& state, RCModeEnum mode) {
	switch(mode) {
		case RCModeEnum::IDLE:
			return new RCIdleMode(state, state.getRCMode());
			break;
		case RCModeEnum::RUDDER:
			return new RCRudderMode(state, state.getRCMode());
			break;
		case RCModeEnum::COURSE:
			return new RCCourseMode(state, state.getRCMode());
			break;
		case RCModeEnum::FAILSAFE:
			return new RCFailsafeMode(state, state.getRCMode());
			break;
		default:
			return new RCIdleMode(state, state.getRCMode());
			break;
	}
}

RCModeBase *RCIdleMode::execute() {
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::IDLE) {
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}

RCModeBase *RCRudderMode::execute() {
	// Write the outgoing rudder command
	_state.rudder->write(_state.rc->getRudder());
	// Set the throttle
	_state.throttle->setThrottle(_state.rc->getThrottle());
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::RUDDER) {
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
	if (!callCount) {
		helm.SetControllerDirection(RUDDER_DIRECTION);
		helm.SetSampleTime(RUDDER_PERIOD);
	}
	// Grab the current orientation and find the heading error for the PID loop
	in = _state.orient->getOrientation()->headingError(_state.rc->getCourse());
	// Execute the PID process
	helm.Compute();	
	// Write the outgoing rudder command
	_state.rudder->write(out);
	// Set the throttle
	_state.throttle->setThrottle(_state.rc->getThrottle());
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::COURSE) {
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}

RCModeBase *RCFailsafeMode::execute() {
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	// Choose the next command
	if (_state.rc->getMode() != RCModeEnum::FAILSAFE) {
		return RCModeBase::factory(_state, _state.rc->getMode());
	}
	return this;
}