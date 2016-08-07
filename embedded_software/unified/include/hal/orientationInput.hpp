/******************************************************************************
 * Hackerboat orientation input module
 * hal/orientationInput.hpp
 * This module reads orientation data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ORIENTATIONINPUT_H
#define ORIENTATIONINPUT_H

#include <string>
#include <stdlib.h>
#include <time.h>
#include "orientation.hpp"
#include "hal/config.h"

class orientationInputClass {
	orientationInputClass(void);			
	orientation getOrientation(void);		/**< Get the last orientation recorded */
	bool execute(void);						/**< Gather input from orientation sensor(s) (meant to be called regularly)	*/
	
	timespec lastInput;						/**< Time that last input was processed 							*/
};

#endif
 