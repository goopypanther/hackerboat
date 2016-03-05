/******************************************************************************
 * Hackerboat Beaglebone State machine mode
 * hackerboatStateMachine.hpp
 * This program is the core vessel state machine module
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "BlackUART.h"
#include "config.h"
#include "stateStructTypes.hpp"
#include "stateMachine.hpp"
#include "logs.hpp"

logError *err = logError::instance();

using namespace BlackLib;

stateTimer::stateTimer (double duration, uint64_t frameTime) {
	this->setDuration(duration, frameTime);
}

void stateTimer::setDuration (double duration, uint64_t frameTime) {
	double period = (frameTime/1e9);
	this->_duration = duration/period;
}

stateMachineBase *boneStartState::execute (void) {
	return new boneSelfTestState(this->_state, this->_ard);
}

stateMachineBase *boneSelfTestState::execute (void) {
	bool passFlag = true;
	
	if ((this->_state.state == BONE_FAULT) ||
		(this->_ard.state == BOAT_FAULT)) {
			return new boneFaultState(this->_state, this->_ard);
	}
	
	this->_state.setState(BONE_SELFTEST);
	
	// check if we got a GNSS fix in the database
	if (this->_state.gps.isValid()) {
		// check if the fix arrived since the beginning of the test phase
		if (this->_state.gps.uTime.tv_sec < _start.tv_sec) {
			_state->removeFault("No GNSS");
		} else {
			passFlag = false;
			_state->insertFault("No GNSS");
		}
		// check if we've heard from the shore since the start of the test phase
		if (_state->lastContact.tv_sec < _start.tv_sec) {
			_state->removeFault("No Shore");
		} else if (!_state->autonomous) {
			passFlag = false;
			_state->insertFault("No Shore");
		}
	} else {
		passFlag = false;
		_state->insertFault("No GNSS");
	}
	
	// if all tests pass, let's do some stuff
	if (passFlag) {
		if (_state->command == BONE_ARMEDTEST) {
			return new boneArmedTestState(this->_state, this->_ard);
		} else if (this->_lastState == BONE_WAYPOINT) {
			return new boneWaypointState(this->_state, this->_ard);
		} else if (this->_lastState == BONE_RETURN) {
			return new boneReturnState(this->_state, this->_ard);
		} else {
			return new boneDisarmedState(this->_state, this->_ard);
		}
	} else if (_count > SELFTEST_FRAMES) {
		if ((_state->faultCount() == 1) && (_state->hasFault("No Shore"))) {
			return new boneNoSignalState(this->_state, this->_ard);
		} else {
			return new boneFaultState(this->_state, this->_ard);
		}
	}
	
	this->_count++;
	return this;
}

stateMachineBase *boneDisarmedState::execute (void) {
	
	if ((this->_state->state == BONE_FAULT) ||
		(this->_ard->state == BOAT_FAULT)) {
			return new boneFaultState(this->_state, this->_ard);
	}
	
	this->_state.setState(BOAT_DISARMED);
	
	// update the start location
	this->launchPoint._lat = this->gps.latitude;
	this->launchPoint._lon = this->gps.longitude;
	
	if ((_ard.state == BOAT_ARMED) || (_state->command == BONE_ARMED)) {
		return new boneArmedState(this->_state, this->_ard);
	} else {
		if ((!this->_state.gps.isValid()) || 
			((_state->uTime.tv_sec - this->_state.gps.uTime.tv_sec) > 180)) {
			_state->insertFault("No GNSS");
			return new boneFaultState(this->_state, this->_ard);
		} 
	}
	
	// check for shore signal
	if ((_state->lastContact.tv_sec + SHORE_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Shore");
		return new boneNoSignalState(this->_state, this->_ard);
	}
	
	// check for Arduino
	if ((_ard->uTime.tv_sec + ARDUINO_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Arduino");
		return new boneFaultState(this->_state, this->_ard);
	}
	
	return this;
}

stateMachineBase *boneArmedState::execute (void) {
	
	if ((this->_state->state == BONE_FAULT) ||
		(this->_ard->state == BOAT_FAULT)) {
			return new boneFaultState(this->_state, this->_ard);
	}
	
	this->_state.setState(BOAT_ARMED);
	
	// check for GNSS
	if ((!this->_state.gps.isValid()) || 
		((_state->uTime.tv_sec - _fix.uTime.tv_sec) > 180)) {
		_state->insertFault("No GNSS");
		return new boneFaultState(this->_state, this->_ard);
	} 
	
	// update the start location
	this->launchPoint._lat = this->gps.latitude;
	this->launchPoint._lon = this->gps.longitude;
	
	// check for disarmed command or signal
	if ((_ard->state == BOAT_DISARMED) || (_state->command == BONE_DISARMED)) {
		return new boneDisarmedState(this->_state, this->_ard);
	}
	
	// check for shore signal
	if ((_state->lastContact.tv_sec + SHORE_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Shore");
		return new boneNoSignalState(this->_state, this->_ard);
	}
	
	// check for Arduino
	if ((_ard->uTime.tv_sec + ARDUINO_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Arduino");
		return new boneFaultState(this->_state, this->_ard);
	}
	
	// check for commands
	if (_state->command == BONE_MANUAL) {
		return new boneManualState(this->_state, this->_ard);
	} else if (_state->command == BONE_WAYPOINT) {
		return new boneWaypointState(this->_state, this->_ard);
	} else if (_state->command == BONE_DISARMED) {
		return new boneDisarmedState(this->_state, this->_ard);
	}
	
	return this;
}
	
stateMachineBase *boneManualState::execute (void) {
	
	if ((this->_state->state == BONE_FAULT) ||
		(this->_ard->state == BOAT_FAULT)) {
			return new boneFaultState(this->_state, this->_ard);
	}
	
	this->_state.setState(BOAT_MANUAL);
	
	// check for GNSS
	if ((!this->_state.gps.isValid()) || 
		((_state->uTime.tv_sec - _fix.uTime.tv_sec) > 180)) {
		_state->insertFault("No GNSS");
		return new boneFaultState(this->_state, this->_ard);
	} 
	
	// check for disarmed command or signal
	if ((_ard->state == BOAT_DISARMED) || (_state->command == BONE_DISARMED)) {
		return new boneDisarmedState(this->_state, this->_ard);
	}
	
	// check for shore signal
	if ((_state->lastContact.tv_sec + SHORE_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Shore");
		return new boneNoSignalState(this->_state, this->_ard);
	}
	
	// check for Arduino
	if ((_ard->uTime.tv_sec + ARDUINO_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Arduino");
		return new boneFaultState(this->_state, this->_ard);
	}
	
	// check for commands
	if (_state->command == BONE_ARMED) {
		return new boneArmedState(this->_state, this->_ard);
	} else if (_state->command == BONE_WAYPOINT) {
		return new boneWaypointState(this->_state, this->_ard);
	} else if (_state->command == BONE_RETURN) {
		return new boneReturnState(this->_state, this->_ard);
	}
	
	return this;
}

stateMachineBase *boneWaypointState::execute (void) {
	
	if ((this->_state->state == BONE_FAULT) ||
		(this->_ard->state == BOAT_FAULT)) {
			return new boneFaultState(this->_state, this->_ard);
	}
	
	this->_state.setState(BOAT_WAYPOINT);
	
	// check for GNSS
	if ((!this->_state.gps.isValid()) || 
		((_state->uTime.tv_sec - _fix.uTime.tv_sec) > 180)) {
		_state->insertFault("No GNSS");
		return new boneFaultState(this->_state, this->_ard);
	} 
	
	// check for disarmed command or signal
	if ((_ard->state == BOAT_DISARMED) || (_state->command == BONE_DISARMED)) {
		return new boneDisarmedState(this->_state, this->_ard);
	}
	
	// check for shore signal
	if ((_state->lastContact.tv_sec + SHORE_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Shore");
		return new boneNoSignalState(this->_state, this->_ard);
	}
	
	// check for Arduino
	if ((_ard->uTime.tv_sec + ARDUINO_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Arduino");
		return new boneFaultState(this->_state, this->_ard);
	}
	
	// load & write navigation data
	_nav->openFile();
	_nav->getLastRecord();
	if (_nav->isValid()) {
		// calculate target heading & throttle
		_ard->headingTarget = _nav->total._bearing + _nav->magCorrection;
		_ard->throttle = (int8_t)(INT8_MAX * (_nav->total.strength/_state->waypointStrengthMax));
		// transmit arduino commands
		_ard->writeThrottle();
		_ard->writeHeadingTarget();
	} 
	_nav->target = _wp;
	_nav->writeRecord();
	_nav->closeFile();
	
	// check waypoint distance
	_wp->openFile();
	_wp->getRecord(_state->waypointNext);
	if (_wp->isValid()) {
		if (_wp->location.distance(_nav->current) < _state->waypointAccuracy) {
			if (_wp->count() > _state->waypointNext) {
				_state->waypointNext++;
			} else {
				switch (_wp->act) {
					case HOME:
						return new boneReturnState(this->_state, this->_ard);
					case CONTINUE:
						_state->waypointNext = 0;
					case STOP:
					default:
						_ard->throttle = 0;
						_ard->writeThrottle();
						return new boneManualState(this->_state, this->_ard);
				}
			}
		}
	}
	_wp->closeFile();
	
	// check incoming commands
	if (_state->command == BONE_ARMED) {
		return new boneArmedState(this->_state, this->_ard);
	} else if (_state->command == BONE_MANUAL) {
		return new boneManualState(this->_state, this->_ard);
	} else if (_state->command == BONE_RETURN) {
		return new boneReturnState(this->_state, this->_ard);
	}
	
	return this;
}