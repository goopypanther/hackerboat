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
		throttleClass();
		throttleClass(adcInputClass& adc);
		bool setThrottle(int throttle);
		int getThrottle();
		double getMotorCurrent();
		double getMotorVoltage();
		bool setADCdevice(adcInputClass& adc);
	private:
		adcInputClass& _adc;
		static vector<std::string> throttleRelays = THROTTLE_RELAY_VECTOR;
};

#endif