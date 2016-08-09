/******************************************************************************
 * Hackerboat ADC input module
 * hal/adcInput.hpp
 * This module reads orientation data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ADCINPUT_H
#define ADCINPUT_H

#include <string>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "hal/config.h"

class adcInputClass {
	adcInputClass(void);			
	vector<int> getvalues (void);			/**< Return the ADC values */
	bool execute(void);						/**< Gather input from adc channels (meant to be called in a loop)	*/
	
	timespec lastInput;						/**< Time that last input was processed 							*/
};

#endif