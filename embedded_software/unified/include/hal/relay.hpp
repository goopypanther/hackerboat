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

#ifndef GPIO_H
#define GPIO_H

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

typedef std::string tuple<double, bool, bool> RelayTuple;

class Relay : public hackerboatStateClass {
	public:
		Relay () = default;
		Relay (std:string name, Pin output, Pin fault, adcInputClass& adc, bool state = false) :
			_name(name), _drive(output), _fault(fault), _adc(adc), _state(state) {
				this->init();
			};
			
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid () {return true;};
		
		bool init();
		bool set();
		bool clear();
		bool isFaulted();
		double current();
		RelayTuple getState();
		std::string& name() {return _name;};
		Pin& output() {return _output;};
		Pin& fault() {return _fault;};
		adcInputClass&& adc() {return _adc;};
		
	private:
		std::string _name;
		Pin _drive;
		Pin _fault;
		bool _state;
		adcInputClass& _adc;
}