/******************************************************************************
 * Hackerboat ADC input module
 * hal/adcInput.cpp
 * This module reads orientation data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <cmath>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include "hal/inputThread.hpp"
#include "hal/config.h"
#include "hal/drivers/adc128d818.hpp"
#include "hal/adcInput.hpp" 
 
bool ADCInput::init() {
	bool result = true;
	
	// set up any internal ADCs and check that we can access the relevant files
	batmonPath = ADC_BATMON_PATH;
	result &= ((access(batmonPath.c_str(), F_OK)) == 0); // check for the existence of the relevant file
	
	// set up the upper external ADC bank
	result &= upper.begin();
	upper.setReferenceMode(reference_mode_t::EXTERNAL_REF);
	upper.setReference(ADC128D818_EXTERNAL_REF);
	
	// set up the lower external ADC bank
	result &= lower.begin();
	lower.setReferenceMode(reference_mode_t::EXTERNAL_REF);
	lower.setReference(ADC128D818_EXTERNAL_REF);
	
	// This populates the various maps
	_raw[ADC_BATMON_NAME] = -1;
	_offsets[ADC_BATMON_NAME] = 0.0;
	_scales[ADC_BATMON_NAME] = 0.00043956;			// default is to scale this value to a raw voltage, i.e. 0-4095 => 0-1.8V
	for (unsigned int i = 0; i < upperChannels.size(); i++) {
		_raw[upperChannels[i]] = -1;
		_offsets[upperChannels[i]] = 0.0;
		_scales[upperChannels[i]] = 0.001221001;	// default is to scale to raw voltage (0-5V)
	}
	for (unsigned int j = 0; j < lowerChannels.size(); j++) {
		_raw[lowerChannels[j]] = -1;
		_offsets[lowerChannels[j]] = 0.0;
		_scales[lowerChannels[j]] = 0.001221001;	// default is to scale to raw voltage (0-5V)
	}
	inputsValid = result;
	return inputsValid;
}

bool ADCInput::begin() {
	if (this->init()) {
		this->myThread = new std::thread (InputThread::InputThreadRunner(this));
		myThread->detach();
		return true;
	}
	return false;
}

bool ADCInput::execute() {
	// grab the lock
	if (!lock && (!lock.try_lock_for(IMU_LOCK_TIMEOUT))) return false;
	bool result = true;
	
	// set the time
	this->setLastInputTime();
	
	// read in the data
	std::vector<int> upperInputs = upper.readAll();
	std::vector<int> lowerInputs = lower.readAll();
	std::ifstream ain;
	char in[5];
	ain.open(batmonPath);
	if (ain.is_open()) {
		ain.get(in, 4);		// value is from 0-1799, so at most four characters
		_raw["battery_mon"] = atoi(in);	// convert millivolts to volts for storage
	} else {
		_raw["battery_mon"] = -1;
		result = false;
	}
	for (unsigned int i = 0; i < upperChannels.size(); i++) {
		_raw[upperChannels[i]] = upperInputs[i];
	}
	for (unsigned int j = 0; j < lowerChannels.size(); j++) {
		_raw[lowerChannels[j]] = lowerInputs[j];
	}
	
	lock.unlock();
	return result;
}

std::map<std::string, double> ADCInput::getScaledValues (void) {
	std::map<std::string, double> out;
	for (auto const &r : _raw) {
		out[r.first] = (r.second + _offsets[r.first]) * _scales[r.first];
	}
	return out;
}

bool ADCInput::setOffsets (std::map<std::string, int> offsets) {
	for (auto const &r : offsets) {
		_offsets[r.first] = r.second;
	}
	return true;
}

bool ADCInput::setScales (std::map<std::string, double> scales) {
	for (auto const &r : scales) {
		_scales[r.first] = r.second;
	}
	return true;
	
}
