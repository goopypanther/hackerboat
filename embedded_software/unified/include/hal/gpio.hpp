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
#include <iostream>
#include <sstream>
#include "hal/config.h"

//typedef tuple<int, int, bool> pinDef;	/**< Pin port, pin, and direction, respectively */

class Pin {
	public:
		Pin () = default;
		Pin (int port, int pin, bool dir, bool state = false) : 
			_port(port), _pin(pin), _dir(dir), _state(state) {
				this->init();
		};
		bool init ();					/**< Initialize the pin. _port and _pin must be set or this returns false. Must be called any time the configuration is changed. */
		bool setPort (int port);		/**< Set the port -- either 8 or 9 */
		bool setPin (int pin);			/**< Set the number of the pin to use */
		void setDir (bool dir) {_dir=dir;};	/**< Set direction -- true is output, false is input. */
		bool writePin (bool state);			/**< Write the pin. Returns true if successful  */
		int readPin () {return this->get();};
		bool set() {return writePin(true);};	/**< Returns true if pin is writeable and write is successful */
		bool clear() {return writePin(false);};	/**< Returns true if pin is writeable and write is successful */					
		int get();						/**< Reads the value of the pin. 1 is high, 0 if low, -1 if error */
		bool getState() {return _state;};
		bool pullUp ();
		bool pullDown ();
		static int getGPIO (int port, int pin);
		
	private:
		std::ostringstream path;
		int _port = -1;
		int _pin = -1;
		int _gpio = -1;
		bool _dir = false;
		bool _state;
};

#endif