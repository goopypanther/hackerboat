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
#include <chrono>
#include <vector>
#include <map>
#include <inttypes.h>
#include "hal/config.h"
#include "hal/gpio.hpp"
#include "hal/adcInput.hpp"
#include <jansson.h>
#include "json_utilities.hpp"

typedef std::tuple<double, bool, bool> RelayTuple;

class Relay {
	public:
		Relay () = default;
		Relay (std::string name, Pin output, Pin fault, ADCInput* adc = NULL, bool state = false) :
			_name(name), _drive(&output), _fault(&fault), _adc(adc), _state(state) {
				this->init();
			};
		Relay (std::string name, Pin *output, Pin *fault, ADCInput* adc = NULL, bool state = false) :
			_name(name), _drive(output), _fault(fault), _adc(adc), _state(state) {
				this->init();
			};
			
		json_t *pack ();							/**< Pack a relay tuple with state, fault, and current output of this relay */
		
		bool init();								/**< Initialize this relay */
		bool set() {return _drive->set();};			/**< Set the output state of this relay to ON */
		bool clear() {return _drive->clear();};		/**< Set the output state of this relay to OFF */
		bool isFaulted() {return _fault->get();};	/**< Check if this relay has a fault */
		double current();							/**< Get the current, in amps */
		RelayTuple getState();						/**< Get the current state of the relay as a tuple */
		std::string& name() {return _name;};		/**< Get a reference to the name of this relay */
		Pin* output() {return _drive;};				/**< Get a pointer to the drive pin */
		bool output(Pin* drive);					/**< Define a new drive pin */
		Pin* fault() {return _fault;};				/**< Get a pointer to the fault pin */
		bool fault(Pin* fault);						/**< Define a new fault pin */
		ADCInput* adc() {return _adc;};				/**< Get a pointer to the ADCInput object this relay uses to measure current. */
		bool adc(ADCInput* adc);					/**< Define a new ADCInput object */
		bool isInitialized () {return initialized;};
		
	private:
		std::string _name;
		Pin* _drive;
		Pin* _fault;
		bool _state;
		ADCInput* _adc = NULL;
		bool initialized = false;
};

class RelayMap {
	public:
		static RelayMap* instance ();						/**< Returns a pointer to the object */
		bool init ();										/**< Initialize all relays */
		Relay& get (std::string name) {return relays->at(name);}	/**< Get a reference to the named relay */
		json_t *pack ();									/**< Pack status for all of relays in the map. */
		bool adc(ADCInput* adc);							/**< Set the ADC for all relays */
		std::map<std::string, Relay> *getmap () {return relays;};
		
	private:
		RelayMap ();										/**< Hark, a singleton! */
		RelayMap (RelayMap const&) = delete;				/**< Hark, a singleton! */
		RelayMap& operator=(RelayMap const&) = delete;		/**< Hark, a singleton! */
		static RelayMap* 			_instance;				/**< Hark, a singleton! */
		std::map<std::string, Relay> *relays;				/**< Named map of all relays */
		bool						initialized = false;	/**< Record whether all relays are initialized */
};

#endif