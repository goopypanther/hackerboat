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
#include "BlackUART/BlackUART.h"
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

bool stateMachineBase::GNSSFail (void) {
	if ((!_state->gps.isValid()) || ((_state->uTime.tv_sec - _state->gps.uTime.tv_sec) > GNSS_TIMEOUT)) {
		_state->insertFault("No GNSS");
		return true;
	} else {
		_state->removeFault("No GNSS");
		return false;
	}
}

bool stateMachineBase::arduinoFail (void) {
	if ((_ard->uTime.tv_sec + ARDUINO_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Arduino");
		return true;
	} else {
		_state->removeFault("No Arduino");
		return false;
	}
}

bool stateMachineBase::shoreFail (void) {
	if ((_state->lastContact.tv_sec + SHORE_TIMEOUT) < _state->uTime.tv_sec) {
		_state->insertFault("No Shore");
		return true;
	} else {
		_state->removeFault("No Shore");
		return false;
	}
}

bool stateMachineBase::isDisarmed (void) {
	return ((_ard->mode == arduinoModeEnum::DISARMED) || (_state->command == boatModeEnum::DISARMED)); 
}

bool stateMachineBase::isFaulted (void) {
	if ((this->_state->mode == boatModeEnum::FAULT) || (this->_ard->mode == arduinoModeEnum::FAULT)) {
		return true;
	} else return false;
}

stateMachineBase *boneStartState::execute (void) {
	return new boneSelfTestState(this->_state, this->_ard);
}

stateMachineBase *boneSelfTestState::execute (void) {
	bool passFlag = true;
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::SELFTEST);
	
	// check if we got a GNSS fix in the database
	if (this->_state->gps.isValid()) {
		// check if the fix arrived since the beginning of the test phase
		if (this->_state->gps.uTime.tv_sec < _start.tv_sec) {
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
		if (_state->command == boatModeEnum::ARMEDTEST) {
			return new boneArmedTestState(this->_state, this->_ard);
		} else if (this->_lastState == boatModeEnum::WAYPOINT) {
			return new boneWaypointState(this->_state, this->_ard);
		} else if (this->_lastState == boatModeEnum::RETURN) {
			return new boneReturnState(this->_state, this->_ard);
		} else {
			return new boneDisarmedState(this->_state, this->_ard);
		}
	} else if ((this->_state->uTime.tv_sec - this->_start.tv_sec) > SELFTEST_DELAY) {
		if ((_state->faultCount() == 1) && (_state->hasFault("No Shore"))) {
			return new boneNoSignalState(this->_state, this->_ard);
		} else {
			return new boneFaultState(this->_state, this->_ard);
		}
	}
	
	return this;
}

stateMachineBase *boneDisarmedState::execute (void) {
	
	// check if we're starting with a fault
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::DISARMED);
	
	// check for GNSS, shore signal, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (shoreFail()) return new boneNoSignalState(this->_state, this->_ard);
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	
	// update the start location
	_state->launchPoint._lat = _state->gps.latitude;
	_state->launchPoint._lon = _state->gps.longitude;
	
	// check if we're arming
	if ((_ard->mode == arduinoModeEnum::ARMED) && (_state->command == boatModeEnum::ARMED)) {
		return new boneArmedState(this->_state, this->_ard);
	} 
	
	return this;
}

stateMachineBase *boneArmedState::execute (void) {
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::ARMED);
	
	// check for GNSS, shore signal, disarmed, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (shoreFail()) return new boneNoSignalState(this->_state, this->_ard);
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	if (isDisarmed()) return new boneDisarmedState(this->_state, this->_ard);
	
	// update the start location
	_state->launchPoint._lat = _state->gps.latitude;
	_state->launchPoint._lon = _state->gps.longitude;
	
	// check for commands
	if (_state->command == boatModeEnum::MANUAL) {
		return new boneManualState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::WAYPOINT) {
		return new boneWaypointState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::DISARMED) {
		return new boneDisarmedState(this->_state, this->_ard);
	}
	
	return this;
}
	
stateMachineBase *boneManualState::execute (void) {
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::MANUAL);
	
	// check for GNSS, shore signal, disarm, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (shoreFail()) return new boneNoSignalState(this->_state, this->_ard);
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	if (isDisarmed()) return new boneDisarmedState(this->_state, this->_ard);
	
	// check for commands
	if (_state->command == boatModeEnum::ARMED) {
		return new boneArmedState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::WAYPOINT) {
		return new boneWaypointState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::RETURN) {
		return new boneReturnState(this->_state, this->_ard);
	}
	
	return this;
}

stateMachineBase *boneWaypointState::execute (void) {
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::WAYPOINT);
	
	// check for GNSS, shore signal, disarm, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (shoreFail() && !(_state->autonomous)) return new boneNoSignalState(this->_state, this->_ard);
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	if (isDisarmed()) return new boneDisarmedState(this->_state, this->_ard);
	
	// load & write navigation data
	_nav.getLastRecord();
	_wp.getRecord(_state->waypointNext);
	_nav.target = _wp;
	if (_nav.isValid()) {
		// calculate target heading & throttle
		_ard->headingTarget = _nav.total._bearing + _nav.magCorrection;
		_ard->throttle = (int8_t)(INT8_MAX * (_nav.total._strength/_state->waypointStrengthMax));
		// transmit arduino commands
		_ard->writeThrottle();
		_ard->writeHeadingTarget();
	} 
	_nav.writeRecord();
	
	// check waypoint distance
	if (_wp.isValid()) {
		if (_wp.location.distance(_nav.current) < _state->waypointAccuracy) {
			if (_wp.countRecords() > _state->waypointNext) {
				_state->waypointNext++;
			} else {
				switch (_wp.getAction()) {
					case waypointClass::action::HOME:
						return new boneReturnState(this->_state, this->_ard);
					case waypointClass::action::CONTINUE:
						_state->waypointNext = 0;
					case waypointClass::action::STOP:
					default:
						_ard->throttle = 0;
						_ard->writeThrottle();
						return new boneManualState(this->_state, this->_ard);
				}
			}
		}
	}
	
	// check incoming commands
	if (_state->command == boatModeEnum::ARMED) {
		return new boneArmedState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::MANUAL) {
		return new boneManualState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::RETURN) {
		return new boneReturnState(this->_state, this->_ard);
	}
	
	return this;
}

stateMachineBase *boneNoSignalState::returnLastState (void) {
	switch (this->_lastState) {
		case boatModeEnum::ARMED:
			return new boneArmedState(this->_state, this->_ard);
		case boatModeEnum::MANUAL:
			return new boneManualState(this->_state, this->_ard);
		case boatModeEnum::WAYPOINT:
			return new boneWaypointState(this->_state, this->_ard);
		default:
			return this;
	}
}

stateMachineBase *boneNoSignalState::execute (void) {
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::NOSIGNAL);
	
	// check for GNSS, shore signal, disarm, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (!shoreFail()) return returnLastState();
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	if (isDisarmed()) return new boneDisarmedState(this->_state, this->_ard);
	
	// check if we've timed out...
	if ((_state->lastContact.tv_sec + RETURN_TIMEOUT) > _state->uTime.tv_sec) {
		if ((this->_lastState == boatModeEnum::MANUAL) || (this->_lastState == boatModeEnum::WAYPOINT)) {
			return new boneReturnState(this->_state, this->_ard);
		}
	}
	
	return this;
}

stateMachineBase *boneReturnState::execute (void) {
	
	if (isFaulted()) return new boneFaultState(this->_state, this->_ard);
	
	this->_state->setMode(boatModeEnum::RETURN);
	
	// check for GNSS, disarm, and arduino
	if (GNSSFail()) return new boneFaultState(this->_state, this->_ard);
	if (arduinoFail()) return new boneFaultState(this->_state, this->_ard);
	if (isDisarmed()) return new boneDisarmedState(this->_state, this->_ard);
	
	// load & write navigation data
	_nav.getLastRecord();
	_nav.target.location = _state->launchPoint;
	if (_nav.isValid()) {
		// calculate target heading & throttle
		_ard->headingTarget = _nav.total._bearing + _nav.magCorrection;
		_ard->throttle = (int8_t)(INT8_MAX * (_nav.total._strength/_state->waypointStrengthMax));
		// transmit arduino commands
		_ard->writeThrottle();
		_ard->writeHeadingTarget();
	} 
	_nav.writeRecord();
	
	// check incoming commands
	if (_state->command == boatModeEnum::ARMED) {
		return new boneArmedState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::MANUAL) {
		return new boneManualState(this->_state, this->_ard);
	} else if (_state->command == boatModeEnum::WAYPOINT) {
		return new boneWaypointState(this->_state, this->_ard);
	}
	
	return this;
}
	
stateMachineBase *boneFaultState::execute (void) {
	
	this->_state->setMode(boatModeEnum::FAULT);
	
	// check for GNSS, arduino, and shore signal
	GNSSFail();
	shoreFail();
	arduinoFail();
	
	// check if there are still faults
	if (!(_state->faultCount())) return new boneDisarmedState(this->_state, this->_ard);
	
	// check if we're getting a command for self test
	if (_state->command == boatModeEnum::SELFTEST) return new boneSelfTestState(this->_state, this->_ard);
	
	return this;
}