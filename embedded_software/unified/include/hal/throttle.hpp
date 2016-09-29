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
#include <vector>
#include "hal/adcInput.hpp"
#include "hal/config.h"
#include "hal/relay.hpp"

class Throttle {
	public:
		Throttle() = default;						
		Throttle(ADCInput* adc) : _adc(adc) {};	/**< Create a motor device with a reference to an ADC input instance for motor current & voltage */
			
		bool setThrottle(int throttle);		/**< Set throttle to the given value. Returns false if the value is outside of the range defined by getMaxThrottle() and getMinThrottle() */
		int getThrottle() {return _throttle;};	/**< Get current throttle position */
		double getMotorCurrent();			/**< Get the current motor current */
		double getMotorVoltage();			/**< Get the current motor voltage */
		bool setADCdevice(ADCInput* adc) {	/**< Set the ADC input thread */
			if (_adc) {
				_adc = adc;
				return true;
			}
			return false;
		}
		int getMaxThrottle() {return throttleMax;};		/**< Get the maximum throttle value */
		int getMinThrottle() {return throttleMin;};		/**< Get the minimum throttle value */
	private:
		int _throttle = 0;
		ADCInput* _adc;
		RelayMap *relays = RelayMap::instance();
		const int throttleMax = THROTTLE_MAX;
		const int throttleMin = THROTTLE_MIN;
};

#endif