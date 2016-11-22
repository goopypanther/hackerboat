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
#include <chrono>
#include <vector>
#include <map>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include "hal/config.h"

class HalTestHarness;

//typedef tuple<int, int, bool> pinDef;	/**< Pin port, pin, and direction, respectively */

class Pin {
	friend class HalTestHarness;
	public:
		Pin () = default;
		Pin (int port, int pin, bool dir, bool state = false) : 
			_port(port), _pin(pin), _dir(dir), _state(state) {
				if (_dir) {
					function = "out";
				} else function = "in";
				this->init();
		};
		bool init ();							/**< Initialize the pin. _port and _pin must be set or this returns false. Must be called any time the configuration is changed. */
		bool setPort (int port);				/**< Set the port -- either 8 or 9 */
		bool setPin (int pin);					/**< Set the number of the pin to use */
		bool setDir (bool dir);					/**< Set direction -- true is output, false is input. */
		bool writePin (bool state);				/**< Write the pin. Returns true if successful  */
		int readPin () {return this->get();};
		bool set() {return writePin(true);};	/**< Returns true if pin is writeable and write is successful */
		bool clear() {return writePin(false);};	/**< Returns true if pin is writeable and write is successful */					
		int get();								/**< Reads the value of the pin. 1 is high, 0 if low, -1 if error */
		bool getState() {return _state;};		/**< Get the state of the pin at the last successful reading */
		bool pullUp ();							/**< Turn on the internal pull-up */
		bool pullDown ();						/**< Turn on the internal pull-up */
		bool floating ();						/**< Turn off internal pull-ups and pull-downs */
		bool isInit() {return _init;};
		
	private:
		int getGPIO (int port, int pin);	/**< Return the internal GPIO number for the pin at the given port and pin number */
		std::string path;
		std::string pinName;
		std::string function = "gpio";
		int _port = -1;
		int _pin = -1;
		int _gpio = -1;
		bool _dir = false;						
		bool _state;
		bool _init = false;
};

#endif