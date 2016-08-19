/******************************************************************************
 * Hackerboat Beaglebone boat modes module
 * boatModes.hpp
 * This is the top-level boat modes
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef BOATMODES_H
#define BOATMODES_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include "hal/config.h"
#include "enumdefs.hpp"
#include "stateMachine.hpp"
#include "boatState.hpp"

class boatModeBaseClass : public stateMachineBaseClass<boatModeEnum, boatStateClass> {
};

class boatStartMode : public boatModeBaseClass {
	boatStartMode (boatStateClass& state) : _state(state),
										   _thisMode(START),
										   _lastMode(NONE) {);
	boatStartMode (boatStateClass& state, boatModeEnum last) : _state(state),
															   _thisMode(START),
															   _lastMode(NONE) {);
};

class boatSelfTestMode : public boatModeBaseClass {
	boatSelfTestMode (boatStateClass& state) : _state(state),
										       _thisMode(SELFTEST),
										       _lastMode(NONE) {);
	boatSelfTestMode (boatStateClass& state, boatModeEnum last) : _state(state),
																  _thisMode(SELFTEST),
																  _lastMode(last) {);
};

#endif