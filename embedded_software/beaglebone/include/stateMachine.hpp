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
 
#ifndef STATEMACHINE_H
#define STATEMACHINE_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "stateStructTypes.hpp"

class stateTimer {
	public:
		stateTimer (double duration, uint64_t frameTime);	/**< Duration in seconds */
		stateTimer (uint32_t duration) {_duration = duration;};	/**< Duration in frames */
		void setDuration (double duration, uint64_t frameTime);
		void setDuration (uint32_t duration) {_duration = duration;};
		void count (void) {_current++;};
		bool isComplete (void) {return (_current >= _duration);};
		void reset (void) {_current = 0;};
	private:
		uint32_t _duration;
		uint32_t _current;
}

class stateMachineBase {
	use boneStateClass;
	public:
		stateMachineBase (boneStateClass *state, arduinoStateClass *ard) {
			_state = state;
			_ard = ard;
			clock_gettime(CLOCK_REALTIME, &_start);
			_lastState = this->_state->state;
		};
		virtual stateMachineBase *execute (void) = 0;
		boneStateClass *getState (void) {return _state};
		arduinoStateClass *getArduino (void) {return _ard}; 
		bool GNSSFail (void);
		bool arduinoFail (void);
		bool shoreFail (void);
		bool isDisarmed (void);
		bool isFaulted (void);
		
	protected:
		boneStateClass 		*_state;
		arduinoStateClass	*_ard;
		timespec			_start;
		boneStateClass::boneStateEnum _lastState;
	
}

class boneStartState : public stateMachineBase {}

class boneSelfTestState : public stateMachineBase {}

class boneDisarmedState : public stateMachineBase {}

class boneArmedState : public stateMachineBase {}

class boneManualState : public stateMachineBase {}

class boneWaypointState : public stateMachineBase {
	private:
		navClass 		_nav(NAV_DB_FILE);
		waypointClass	_wp(WP_DB_FILE);
}

class boneNoSignalState : public stateMachineBase {
	private:
		stateMachineBase *returnLastState (void);
}

class boneReturnState : public stateMachineBase {
	private:
		navClass _nav(NAV_DB_FILE);
}

class boneArmedTestState : public stateMachineBase {}

class boneFaultState : public stateMachineBase {}

#endif /* STATEMACHINE_H */