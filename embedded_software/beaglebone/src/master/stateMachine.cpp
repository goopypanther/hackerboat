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

using namespace BlackLib;

stateTimer::stateTimer (double duration, uint64_t frameTime) {
	this->setDuration(duration, frameTime);
}

void stateTimer::setDuration (double duration, uint64_t frameTime) {
	double period = (frameTime/1e9);
	this->_duration = duration/period;
}

stateMachineBase *boneStartState::execute (void) {
	return new boneSelfTestState(this->_state);
}

stateMachineBase *boneSelfTestState::execute (void) {
	bool passFlag = true;
	
	this->_state->state = BONE_SELFTEST;
	
	if (_ard.populate()) {	// first, check if the Arduino is present and talking
		_state->removeFault("No Arduino");
		if (_ard.state == BOAT_FAULT) {
			passFlag = false;
			_state->insertFault("Arduino Fault");
		} else {
			_state->removeFault("Arduino Fault");
		}
		// check if we got a GNSS fix in the database
		if (_fix.openFile() && _fix.getLastRecord() && _fix.isValid()) {
			// check if the fix arrived since the beginning of the test phase
			if (_fix.uTime.tv_sec < _start.tv_sec) {
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
	} else {
		passFlag = false;
		_state->insertFault("No Arduino"); 
	}
	
	// if all tests pass, let's do some stuff
	if (passFlag) {
		if (_state->command == BONE_ARMEDTEST) {
			return new boneArmedTestState(this->_state);
		} else if (this->_lastState == BONE_WAYPOINT) {
			return new boneWaypointState(this->_state);
		} else if (this->_lastState == BONE_RETURN) {
			return new boneReturnState(this->_state);
		} else {
			return new boneDisarmedState(this->_state);
		}
	} else if (_count > SELFTEST_FRAMES) {
		if ((_state->faultCount() == 1) && (_state->hasFault("No Shore"))) {
			return new boneNoSignalState(this->_state);
		} else {
			return new boneFaultState(this->_state);
		}
	}
	
	this->_count++;
	return this;
}

stateMachineBase *boneDisarmedState::execute (void) {
	
	this->_state->state = BOAT_DISARMED;
	
	if (_ard.populate()) {
		if ((_ard.state == BOAT_ARMED) && (_state->command == BONE_ARMED)) {
			return new boneArmedState(this->_state);
		} else {
			if ((!_fix.getLastRecord()) || 
				((_state->uTime.tv_sec - _fix.uTime.tv_sec) > 180)) {
				_state->insertFault("No GNSS");
				return new boneFaultState(this->_state);
			} 
		}
	} else {
		_state->insertFault("No Arduino");
		return new boneFaultState(this->_state);
	}
	return this;
}

