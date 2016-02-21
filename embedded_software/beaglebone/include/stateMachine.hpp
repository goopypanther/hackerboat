/******************************************************************************
 * Hackerboat Beaglebone State machine mode
 * hackerboatStateMachine.cpp
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
#include "config.h"
#include "stateStructTypes.hpp"

class stateMachineBase {
	use boneStateClass;
	public:
		stateMachineBase (boneStateClass *state) {_state = state};
		virtual stateMachineBase *execute (void);
		boneStateClass *getState (void) {return &_state};
		
	protected:
		boneStateClass *_state;
	
}

class boneStartState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneSelfTestState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneDisarmedState : public stateMachineBase {
	public:
		stateMachineBase *execute (void);
}

class boneFaultState : public stateMachineBase {
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

#endif /* STATEMACHINE_H */