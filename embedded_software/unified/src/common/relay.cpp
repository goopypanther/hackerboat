/******************************************************************************
 * Hackerboat Relay module
 * hal/relay.cpp
 * This module reads and writes relays pins as well as keeping track 
 * of current flow and fault status.
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
#include <inttypes.h>
#include "hal/config.h"
#include "hal/gpio.hpp"
#include "hal/adcInput.hpp"
#include "hal/relay.hpp"
#include <jansson.h>
#include "json_utilities.hpp"

json_t *Relay::pack () {
	json_t* output = json_object();
	int packResult = 0;
	RelayTuple thisrelay = getState();
	packResult += json_object_set_new(output, "current", json(std::get<0>(thisrelay)));
	packResult += json_object_set_new(output, "output", json(std::get<1>(thisrelay)));
	packResult += json_object_set_new(output, "fault", json(std::get<2>(thisrelay)));
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool Relay::init() {
	if (initialized) return true;
	if ((this->_adc->getScaledValues().count(this->_name)) > 0) {
		if (_drive->init() && _fault->init() && _drive->writePin(_state)) {
			initialized = true;
			return true;
		}
	}	
	return false;
}

double Relay::current() const {
	return this->_adc->getScaledValues().at(this->_name);
}

RelayTuple Relay::getState() {
	RelayTuple thisrelay;
	std::get<0>(thisrelay) = this->current();
	if (_drive->get() > 0) {
		std::get<1>(thisrelay) = true;
		_state = true;
	} else std::get<1>(thisrelay) = false;
	if (_fault->get()) {
		std::get<2>(thisrelay) = true;
	} else std::get<2>(thisrelay) = false;
	return thisrelay;
}	

bool Relay::output(Pin* drive) {
	if (drive) {
		_drive = drive;
		initialized = false;
		return true;
	}
	return false;
}

bool Relay::fault(Pin* fault) {
	if (fault) {
		_fault = fault;
		initialized = false;
		return true;
	}
	return false;
}

bool Relay::adc(ADCInput* adc) {
	if (adc) {
		_adc = adc;
		initialized = false;
		return true;
	}
	return false;
}

RelayMap::RelayMap () {
	relays = new std::map<std::string, Relay> RELAY_MAP_INITIALIZER; 
}

bool RelayMap::init() {
	bool result = true;
	for (auto &r : *relays) {
		result &= r.second.init();
	}
	initialized = result;
	return result;
}

json_t* RelayMap::pack () {
	json_t* output = json_object();
	int packResult = 0;
	for (auto &r : *relays) {
		packResult += json_object_set_new(output, std::get<0>(r).c_str(), std::get<1>(r).pack());
	}
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool RelayMap::adc(ADCInput* adc) {
	bool result = true;
	for (auto &r : *relays) {
		result &= r.second.adc(adc);
	}
	initialized = false;
	return result;
}
