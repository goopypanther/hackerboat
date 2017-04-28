/******************************************************************************
 * Hackerboat Beaglebone boat modes module
 * autoModes.cpp
 * This is the autonomous modes
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
#include "hackerboatRoot.hpp"
#include "hal/relay.hpp"
#include "hal/gpio.hpp"
#include "pid.hpp"
#include "rcModes.hpp"
#include "easylogging++.h"

AutoModeBase* AutoModeBase::factory(BoatState& state, AutoModeEnum mode) {
	switch (mode) {
		case AutoModeEnum::IDLE:
			LOG(DEBUG) << "Creating new Idle auto mode object";
			return new AutoIdleMode(state, state.getAutoMode());
			break;
		case AutoModeEnum::WAYPOINT:
			LOG(DEBUG) << "Creating new Waypoint auto mode object";
			return new AutoWaypointMode(state, state.getAutoMode());
			break;
		case AutoModeEnum::RETURN:
			LOG(DEBUG) << "Creating new Return auto mode object";
			return new AutoReturnMode(state, state.getAutoMode());
			break;
		case AutoModeEnum::ANCHOR:
			LOG(DEBUG) << "Creating new Anchor auto mode object";
			return new AutoAnchorMode(state, state.getAutoMode());
			break;
		default:
			LOG(DEBUG) << "Creating new Idle auto mode object by default";
			return new AutoIdleMode(state, state.getAutoMode());
	}
}

AutoModeBase* AutoIdleMode::execute() {
	if (this->callCount == 0) {
		LOG(INFO) << "Starting auto idle mode";
	}
	callCount++;
	// Write the outgoing rudder command
	_state.rudder->write(0);
	// Set the throttle
	_state.throttle->setThrottle(0);
	// Choose the next command
	if (_state.getAutoMode() != AutoModeEnum::IDLE) {
		LOG(DEBUG) << "Leaving auto idle mode for " << _state.autoModeNames.get(_state.getAutoMode()) << " by command";
		return AutoModeBase::factory(_state, _state.getAutoMode());
	}
	return this;
}

AutoModeBase* AutoWaypointMode::execute() {
	if ((helm.GetKp() != std::get<0>(_state.K)) ||
		(helm.GetKi() != std::get<1>(_state.K)) ||
		(helm.GetKd() != std::get<2>(_state.K))) {
			helm.SetTunings(std::get<0>(_state.K), 
							std::get<1>(_state.K), 
							std::get<2>(_state.K));
		}
	if (this->callCount == 0) {
		LOG(INFO) << "Starting auto waypoint mode";
	}
	callCount++;
	
	// get course to next waypoint
	Location target = _state.waypointList.getWaypoint();
	
	// get course to the next waypoint
	double targetCourse = _state.lastFix.fix.bearing(target);
	
	// apply dodge functionality, if implemented (this is currently a null)
	
	// operate helm
	this->in = _state.orient->getOrientation()->makeTrue().headingError(targetCourse);
	LOG_EVERY_N(100, DEBUG) << "True Heading: " << _state.orient->getOrientation()->makeTrue() 
							<< ", Target Course: " << targetCourse << ", Waypoint: " 
							<< _state.waypointList.current() << ": " << _state.waypointList.getWaypoint();
	helm.Compute();
	_state.rudder->write(this->out);
	LOG_EVERY_N(100, DEBUG) << "Rudder command: " << to_string(this->out);
	
	// set the throttle
	_state.throttle->setThrottle(this->throttleSetting);
	
	// check if we've arrived at the next waypoint
	if (_state.lastFix.fix.distance(target) < AUTO_WAYPOINT_TOL) {
		LOG(INFO) << "Incrementing waypoint from waypoint " << to_string(_state.waypointList.current());
		if (!_state.waypointList.increment()) {	// if this returns false, it means we got to the end of the waypoint list with and end action other that RETURN
			switch (_state.waypointList.getAction()) {
				case WaypointActionEnum::RETURN:
					LOG(INFO) << "Reached end of waypoint list, returning.";
					return new AutoReturnMode(_state, _state.getAutoMode());
					break;
				case WaypointActionEnum::ANCHOR:
					LOG(INFO) << "Reached end of waypoint list, anchoring.";
					return new AutoAnchorMode(_state, _state.getAutoMode());
					break;
				default:
					LOG(INFO) << "Reached end of waypoint list, going to idle mode.";
					return new AutoIdleMode(_state, _state.getAutoMode());
			}
		} else {
			LOG(INFO) << "New waypoint is #" << to_string(_state.waypointList.current())
						<< " at " << _state.waypointList.getWaypoint();
		}
	}
	
	// check for commands 
	if (_state.getAutoMode() != AutoModeEnum::WAYPOINT) {
		LOG(INFO) << "Leaving auto waypoint mode for " << _state.autoModeNames.get(_state.getAutoMode()) << " by command";
		return AutoModeBase::factory(_state, _state.getAutoMode());
	}
	return this;
}


AutoModeBase* AutoReturnMode::execute() {
	if ((helm.GetKp() != std::get<0>(_state.K)) ||
		(helm.GetKi() != std::get<1>(_state.K)) ||
		(helm.GetKd() != std::get<2>(_state.K))) {
			helm.SetTunings(std::get<0>(_state.K), 
							std::get<1>(_state.K), 
							std::get<2>(_state.K));
		}
	if (this->callCount == 0) {
		LOG(INFO) << "Starting auto return mode, headed for " << _state.launchPoint;
	}
	callCount++;
	
	// get course to next waypoint
	Location target = _state.launchPoint;
	
	// get course to the next waypoint
	double targetCourse = _state.lastFix.fix.bearing(target);
	
	// apply dodge functionality, if implemented (this is currently a null)
	
	// operate helm
	this->in = _state.orient->getOrientation()->makeTrue().headingError(targetCourse);
	LOG_EVERY_N(100, DEBUG) << "True Heading: " << _state.orient->getOrientation()->makeTrue()
							<< ", Target Course: " << to_string(targetCourse) << ", Target: " 
							<< _state.launchPoint;
	helm.Compute();
	_state.rudder->write(this->out);
	LOG_EVERY_N(100, DEBUG) << "Rudder command: " << to_string(this->out);
	
	// set the throttle
	_state.throttle->setThrottle(this->throttleSetting);
	
	// check if we've arrived at the origin
	if (_state.lastFix.fix.distance(target) < AUTO_WAYPOINT_TOL) {
		LOG(INFO) << "Arrived at origin point, anchoring";
		return new AutoAnchorMode(_state, _state.getAutoMode());
	}
	
	// check for commands 
	if (_state.getAutoMode() != AutoModeEnum::RETURN) {
		LOG(DEBUG) << "Leaving auto return mode for " << _state.autoModeNames.get(_state.getAutoMode()) << " by command";
		return AutoModeBase::factory(_state, _state.getAutoMode());
	}
	return this;
}

AutoModeBase* AutoAnchorMode::execute() {
	if ((helm.GetKp() != std::get<0>(_state.K)) ||
		(helm.GetKi() != std::get<1>(_state.K)) ||
		(helm.GetKd() != std::get<2>(_state.K))) {
			helm.SetTunings(std::get<0>(_state.K), 
							std::get<1>(_state.K), 
							std::get<2>(_state.K));
		}
	if (!callCount) {
		_state.anchorPoint = _state.lastFix.fix;
		LOG(INFO) << "Anchoring at " << _state.anchorPoint;
	}
	callCount++;
	
	// get the bearing and distance to the anchor point
	double headingError = _state.orient->getOrientation()->makeTrue().headingError(_state.lastFix.fix.bearing(_state.anchorPoint));
	double distance = _state.lastFix.fix.distance(_state.anchorPoint);
	
	// determine whether the target point is forward or aft of current position
	
	if (distance < ANCHOR_DEADBAND) {
		this->in = 0;
		this->throttleSetting = 0;
	} else {
		this->throttleSetting = round(distance*ANCHOR_THROTTLE_GAIN);
		if (this->throttleSetting > THROTTLE_MAX) {
			this->throttleSetting = THROTTLE_MAX;
		}
		if (abs(headingError) < 90.0) {
			this->in = headingError;
		} else if (headingError > 0.0) {
			this->in = headingError - 180.0;	// reverse course, since we'll be backing up
			this->throttleSetting *= -1;		// reverse the throttle
		} else {
			this->in = headingError + 180.0;	// reverse course, since we'll be backing up
			this->throttleSetting *= -1;		// reverse the throttle
		}
	}

	// operate the helm & throttle
	helm.Compute();
	_state.rudder->write(this->out);
	_state.throttle->setThrottle(this->throttleSetting);
	LOG_EVERY_N(100, DEBUG) << "Rudder command: " << to_string(this->out);
	LOG_EVERY_N(100, DEBUG) << "Throttle command: " << to_string(this->throttleSetting);
	
	// check for commands
	if (_state.getAutoMode() != AutoModeEnum::ANCHOR) {
		LOG(DEBUG) << "Leaving auto return mode for " << _state.autoModeNames.get(_state.getAutoMode()) << " by command";
		return AutoModeBase::factory(_state, _state.getAutoMode());
	}
	return this;
}
