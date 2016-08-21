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

/**
 * @brief A template for a state machine with the given mode enum T and state vector reference U&.
 */

template <typename T, typename U> class stateMachineBaseClass {
	public:
		virtual stateMachineBaseClass (U& state) = 0;			/**< Create a state object with the given state vector and the default mode (typically NONE) */
		virtual stateMachineBaseClass (U& state, T last) = 0;	/**< Create a state object with the given state vector and last mode. */
		virtual stateMachineBaseClass& execute() = 0;			/**< Execute one step of the state machine */
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