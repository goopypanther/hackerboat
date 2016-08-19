/******************************************************************************
 * Hackerboat Beaglebone State machine module
 * stateMachine.hpp
 * This is the core vessel state machine module
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
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"

template <typename T, typename U> class stateMachineBaseClass {
	public:
		virtual stateMachineBaseClass (U& state) = 0;
		virtual stateMachineBaseClass (U& state, T last) = 0;
		virtual stateMachineBaseClass& execute() = 0;
		U& getState() {return _state;}
		T getMode() {return _thisMode;}
		T getLastMode() {return _lastMode;}
		
	protected:
		const U& _state;
		std::chrono::time_point<std::chrono::system_clock> start;
		const T _thisMode;
		const T _lastMode;
	
};

#endif  