/******************************************************************************
 * Hackerboat throttle module
 * hal/throttle.cpp
 * This module sets the throttle
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "hal/adcInput.hpp"
#include "hal/config.h"
#include "hal/relay.hpp"
#include "hal/throttle.hpp"
#include "easylogging++.h"

bool Throttle::setThrottle(int throttle) {
	if ((throttle < throttleMin) || (throttle > throttleMax)) return false;
	_throttle = throttle;
	bool result = true;
	try {
		if (_throttle >= 0) {
			result &= relays->get("DIR").clear();
			LOG(DEBUG) << "Setting throttle forward";
		} else {
			result &= relays->get("DIR").set();
			LOG(DEBUG) << "Setting throttle reverse";
		}
	} catch (...) {
		LOG(WARNING) << "Failed at direction setting" << std::endl;
		return false;
	}
	switch (abs(_throttle)) {
		case 5:
			LOG(DEBUG) << "Setting throttle to 5";
			result &= relays->get("RED").set();
			result &= relays->get("WHT").set();
			result &= relays->get("YLW").set();
			result &= relays->get("REDWHT").set();
			result &= relays->get("YLWWHT").set();
			break;
		case 4:
			LOG(DEBUG) << "Setting throttle to 4";
			result &= relays->get("RED").clear();
			result &= relays->get("WHT").clear();
			result &= relays->get("YLW").set();
			result &= relays->get("REDWHT").set();
			result &= relays->get("YLWWHT").clear();
			break;
		case 3:
			LOG(DEBUG) << "Setting throttle to 3";
			result &= relays->get("RED").clear();
			result &= relays->get("WHT").set();
			result &= relays->get("YLW").set();
			result &= relays->get("REDWHT").clear();
			result &= relays->get("YLWWHT").clear();
			break;
		case 2:
			LOG(DEBUG) << "Setting throttle to 2";
			result &= relays->get("RED").clear();
			result &= relays->get("WHT").set();
			result &= relays->get("YLW").clear();
			result &= relays->get("REDWHT").clear();
			result &= relays->get("YLWWHT").set();
			break;
		case 1:
			LOG(DEBUG) << "Setting throttle to 1";
			result &= relays->get("RED").clear();
			result &= relays->get("WHT").set();
			result &= relays->get("YLW").clear();
			result &= relays->get("REDWHT").clear();
			result &= relays->get("YLWWHT").clear();
			break;
		case 0:
		default:
			LOG(DEBUG) << "Setting throttle to off";
			result &= relays->get("RED").clear();
			result &= relays->get("WHT").clear();
			result &= relays->get("YLW").clear();
			result &= relays->get("REDWHT").clear();
			result &= relays->get("YLWWHT").clear();
			break;
	}
	return result;
}

double Throttle::getMotorCurrent() {
	if (_adc) {
		LOG(DEBUG) << "Motor current is: " << std::to_string(_adc->getScaledValues().at("mot_i")); 
		return _adc->getScaledValues().at("mot_i");
	} 
	return NAN;
}

double Throttle::getMotorVoltage() {
		LOG(DEBUG) << "Motor voltage is: " << std::to_string(_adc->getScaledValues().at("mot_v"));
	if (_adc) {
		return _adc->getScaledValues().at("mot_v");
	} 
	return NAN;
}