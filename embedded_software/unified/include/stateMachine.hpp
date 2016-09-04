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
#include "hackerboatRoot.hpp"

/**
 * @brief A template for a state machine with the given mode enum T and state vector reference U&.
 */

template <typename T, typename U> class StateMachineBase {
	public:
		StateMachineBase (U& state, T last, T thisMode) :	/**< Create a state object with the given state vector and last mode. */
			_state(state), _lastMode(last), _thisMode(thisMode),
			start(std::chrono::system_clock::now()) {};
		virtual StateMachineBase* execute() = 0;			/**< Execute one step of the state machine */
		U& getState() {return _state;}						/**< Get the state vector */
		T getMode() {return _thisMode;}						/**< Get the current mode */
		T getLastMode() {return _lastMode;}					/**< Get the last mode */
		virtual ~StateMachineBase () {};					
		
	protected:
		int callCount = 0;
		const U& _state;
		const sysclock start;
		const T _thisMode;
		const T _lastMode;
	
};

#endif  
