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
		};
		virtual stateMachineBase *execute (void) = 0;
		boneStateClass *getState (void) {return &_state};
		
	protected:
		boneStateClass 		*_state;
		gpsFixClass			_fix(GPS_DB_FILE, strlen(GPS_DB_FILE));
		arduinoStateClass	*_ard;
	
}

class boneStartState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneSelfTestState : public stateMachineBase {
	public:
		boneSelfTestState (boneStateClass *state, arduinoStateClass *ard) {
			_state = state;
			_ard = ard;
			clock_gettime(CLOCK_REALTIME, &_start);
			_lastState = this->_state->state;
		};
		stateMachineBase *execute (void);
	private:
		timespec			_start;
		uint32_t			_count = 0;
		boneStateClass::boneStateEnum _lastState;
}

class boneDisarmedState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneArmedState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneManualState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneWaypointState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneNoSignalState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneReturnState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneArmedTestState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneFaultState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

#endif /* STATEMACHINE_H */