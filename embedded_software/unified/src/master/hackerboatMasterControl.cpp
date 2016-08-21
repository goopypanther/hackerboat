/******************************************************************************
 * Hackerboat Beaglebone Master Control program
 * hackerboatMasterControl.cpp
 * This program is the core vessel control program
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
 
#include "hal/config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include "logs.hpp"
#include "boatState.hpp"
#include "boatModes.hpp"
#include "autoModes.hpp"
#include "rcModes.hpp"
#include "navModes.hpp"
#include "stateMachine.hpp"
#include "location.hpp"
#include "orientation.hpp"
#include "pid.hpp"

#include "hal/adcInput.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/lights.hpp"
#include "hal/orientationInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/servo.hpp"
#include "hal/throttle.hpp"


void inputBB (boatStateClass &state, long stepNum);
void outputBB (boatStateClass &state, long stepNum);

static logError *err = logError::instance();

int main (void) {
	
	return 0;
}
