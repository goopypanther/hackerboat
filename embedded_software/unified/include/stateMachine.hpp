/******************************************************************************
 * Hackerboat Beaglebone State machine module
 * stateMachine.hpp
 * This program is the core vessel state machine module
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef STATEMACHINE_H
#define STATEMACHINE_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "boatState.hpp"
#include "enumdefs.hpp"

template <typename T> class stateMachineBaseClass {
	public:
		stateMachineBaseClass (boatStateClass& state);
		stateMachineBaseClass (boatStateClass& state, T last);
		virtual stateMachineBaseClass& execute() = 0;
		boatStateClass& getState() {return _state;}
		T getMode() {return _thisMode;}
		T getLastMode() {return _lastMode;}
		
	protected:
		boatStateClass &_state;
		std::chrono::time_point<std::chrono::system_clock> start;
		T _thisMode;
		T _lastMode;
	
};

#endif  