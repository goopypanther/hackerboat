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
#include <chrono>
#include <vector>
#include <map>
#include <inttypes.h>
#include <cmath>
#include <tuple>
#include "hal/config.h"
#include "hal/gpio.hpp"
#include "hal/adcInput.hpp"
#include "hal/relay.hpp"
#include "rapidjson/rapidjson.h"
#include "easylogging++.h"

using namespace rapidjson;
using namespace std;

Value Relay::pack () {
	Value d;
	int packResult = 0;
	RelayTuple thisrelay = getState();
	packResult += isfinite(get<0>(thisrelay)) ? HackerboatState::PutVar("current", get<0>(thisrelay), d) : 
												HackerboatState::PutVar("current", d);
	packResult += HackerboatState::PutVar("drive", get<1>(thisrelay), d);
	packResult += HackerboatState::PutVar("fault", get<2>(thisrelay), d);
	
	return d;
}

bool Relay::init() {
	if (this->initialized) {
		LOG(DEBUG) << "Attempting to intialize a previously initialized relay object " << this->_name;
		return true;
	}
	if ((this->_adc) && !((this->_adc->getScaledValues().count(this->_name)) > 0)) {
		LOG(ERROR) << "Failed to obtain current ADC channel for relay " << this->_name;
		return false; 
	}
	this->initialized = true;
	this->initialized &= _drive->init();
	this->initialized &= _fault->init();
	this->initialized &= _drive->writePin(_state);
	return this->initialized;
}

double Relay::current() {
	if (this->_adc) {
		return this->_adc->getScaledValues().at(this->_name);
	}
	return NAN;
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
		LOG(DEBUG) << "Setting drive pin of relay " << this->_name;
 		_drive = drive;
		initialized = false;
		return true;
	}
	return false;
}

bool Relay::fault(Pin* fault) {
	if (fault) {
		LOG(DEBUG) << "Setting fault pin of relay " << this->_name;
		_fault = fault;
		initialized = false;
		return true;
	}
	return false;
}

bool Relay::adc(ADCInput* adc) {
	if (adc) {
		LOG(DEBUG) << "Setting ADCInput object of relay " << this->_name;
		_adc = adc;
		initialized = false;
		return true;
	}
	return false;
}

RelayMap* RelayMap::_instance = new RelayMap();

RelayMap::RelayMap () {
	relays = new std::map<std::string, Relay*>;
	
}

RelayMap *RelayMap::instance () {
	return RelayMap::_instance;
}

bool RelayMap::init() {
	bool result = true;
	for (auto& a : Conf::get()->relayInit()) {
		RelaySpec r = a.second;
		relays->emplace(a.first, 
						new Relay(a.first, new Pin(std::get<1>(r), std::get<2>(r), true),
						new Pin(std::get<3>(r), std::get<4>(r), false)));
	}
	for (auto &r : *relays) {
		result &= r.second->init();
	}
	initialized = result;
	return result;
}

Value RelayMap::pack () {
	Value d;
	int packResult = 0;
	for (auto &r : *relays) {
		packResult += HackerboatState::PutVar(r.first.c_str(), r.second->pack(), d);
	}

	return d;
}

bool RelayMap::adc(ADCInput* adc) {
	bool result = true;
	for (auto &r : *relays) {
		result &= r.second->adc(adc);
	}
	initialized = false;
	return result;
}
