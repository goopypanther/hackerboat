/******************************************************************************
 * Hackerboat GPIO module
 * hal/gpio.hpp
 * This module reads and writes GPIO pins
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

//typedef tuple<int, int, bool> pinDef;	/**< Pin port, pin, and direction, respectively */

class Pin {
	public:
		Pin () = default;
		Pin (int port, int pin, bool dir, bool state = false) :
			_port(port), _pin(pin), _dir(dir), _state(false) {
				this->init();
		};
		bool init ();
		bool setPort (int port);
		bool setPin (int pin);
		void setDir (bool dir);
		void writePin (bool state) {
			(state)?(this->set()):(this->clear());
		};
		bool readPin () {return this->get();};
		bool set();
		bool clear();
		bool get();
		
	private:
		std::string path;
		int _port;
		int _pin;
		bool _dir;
		bool _state;
};

#endif