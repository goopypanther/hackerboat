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
#include <unistd.h>
#include "hal/inputThread.hpp"
#include "hal/config.h"
#include "hal/drivers/adc128d818.hpp"
#include "hal/adcInput.hpp" 
#include "easylogging++.h"
#include "configuration.hpp"
 
ADCInput::ADCInput(void) {
	period = Conf::get()->adcReadPeriod();
}
 
bool ADCInput::init() {
	bool result = true;
	
	LOG(INFO) << "Initializing ADC subsystem";
	// set up any internal ADCs and check that we can access the relevant files
	batmonPath = Conf::get()->batmonPath();
	result &= ((access(batmonPath.c_str(), F_OK)) == 0); // check for the existence of the relevant file
	LOG(DEBUG) << "Result of initializing battery monitor; " << result;
	
	// set up the upper external ADC bank
	result &= upper.begin();
	upper.setReferenceMode(reference_mode_t::EXTERNAL_REF);
	upper.setReference(Conf::get()->adcExternRefVolt());
	LOG(DEBUG) << "Result of initializing upper ADC bank; " << result;
	
	// set up the lower external ADC bank
	result &= lower.begin();
	lower.setReferenceMode(reference_mode_t::EXTERNAL_REF);
	lower.setReference(Conf::get()->adcExternRefVolt());
	LOG(DEBUG) << "Result of initializing lower ADC bank; " << result;
	
	// This populates the various maps
	_raw[Conf::get()->batmonName()] = -1;
	_offsets[Conf::get()->batmonName()] = 0.0;
	_scales[Conf::get()->batmonName()] = 0.0043956;			// default is to scale this value to a battery voltage, i.e. 0-4095 => 0-18V
													// Note that this needs to be calibrated
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
		LOG(DEBUG) << "ADC subsystem started";
		return true;
	}
	LOG(FATAL) << "Unable to initialize ADC subsystem";
	return false;
}

bool ADCInput::execute() {
	// grab the lock
	//if (!lock && (!lock.try_lock_for(IMU_LOCK_TIMEOUT))) return false;
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
		ain.get(in, 5);		// value is from 0-4095, so at most four characters, but we have to grab at least 5 to get it all
		_raw[Conf::get()->batmonName()] = atoi(in);	
	} else {
		_raw[Conf::get()->batmonName()] = -1;
		result = false;
	}
	for (unsigned int i = 0; i < upperChannels.size(); i++) {
		_raw[upperChannels[i]] = upperInputs[i];
	}
	for (unsigned int j = 0; j < lowerChannels.size(); j++) {
		_raw[lowerChannels[j]] = lowerInputs[j];
	}
	
	//lock.unlock();
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
