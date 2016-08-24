/******************************************************************************
 * Hackerboat Relay module
 * hal/relay.hpp
 * This module reads and writes relays pins as well as keeping track 
 * of current flow and fault status.
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef RELAY_H
#define RELAY_H

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
#include "hackerboatRoot.hpp"
#include "adcInput.hpp"

typedef tuple<double, bool, bool> RelayTuple;

class Relay : public HackerboatState {
	public:
		Relay () = default;
		Relay (std::string name, Pin output, Pin fault, ADCInput& adc, bool state = false) :
			_name(name), _drive(output), _fault(fault), _adc(adc), _state(state) {
				this->init();
			};
			
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid () {return true;};
		
		bool init();
		bool set() {return _drive.set();};
		bool clear() {return _drive.clear();};
		bool isFaulted() {return _fault.get();};
		double current();
		RelayTuple getState();
		std::string& name() {return _name;};
		Pin& output() {return _drive;};
		Pin& fault() {return _fault;};
		ADCInput& adc() {return _adc;};
		
	private:
		std::string _name;
		Pin _drive;
		Pin _fault;
		bool _state;
		ADCInput& _adc;
};

#endif