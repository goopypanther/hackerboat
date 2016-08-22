/******************************************************************************
 * Hackerboat throttle module
 * hal/throttle.hpp
 * This module sets the throttle
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef THROTTLE_H
#define THROTTLE_H

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "hal/adcInput.hpp"
#include "hal/config.h"

class throttleClass {
	public:
		throttleClass() = default;						
		throttleClass(adcInputClass& adc);		/**< Create a motor device with a reference to an ADC input instance for motor current & voltage */
			
		bool setThrottle(int throttle);			/**< Set throttle to the given value. Returns false if the value is outside of the range defined by getMaxThrottle() and getMinThrottle() */
		int getThrottle();						/**< Get current throttle position */
		double getMotorCurrent();				/**< Get the current motor current */
		double getMotorVoltage();				/**< Get the current motor voltage */
		bool setADCdevice(adcInputClass& adc);	/**< Set the ADC input thread */
		int getMaxThrottle();					/**< Get the maximum throttle value */
		int getMinThrottle();					/**< Get the minimum throttle value */
	private:
		int _throttle = 0;
		adcInputClass& _adc;
		const vector<std::string> throttleRelays = THROTTLE_RELAY_VECTOR;
};

#endif