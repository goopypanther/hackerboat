/******************************************************************************
 * Hackerboat GPIO module
 * hal/gpio.cpp
 * This module reads and writes GPIO pins
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
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
#include <iostream>
#include <sstream>
#include "hal/config.h"
#include "hal/gpio.hpp"
#include <fcntl.h>  
#include <unistd.h>
#include <fstream>

using namespace std;

bool Pin::init () {
	_gpio = getGPIO(_port, _pin);
	if (_gpio < 0) return false;
	
	pinName = "P" + _port;
	pinName += "_" + _pin;
	
	// export the gpio so we can manipulate it
	path = "/sys/class/gpio/gpio" + _gpio;
	if (access(path.c_str(), F_OK)) {	// returns 0 if it exists, and therefore skips this step
		std::string cmd;
		cmd = "echo " + _gpio;
		cmd += " > /sys/class/gpio/export\n"; 
		if (system(cmd.c_str()) != 0) return false;
	}
	
	// use config-pin to set the pinmux
	std::string pinmux = CONFIG_PIN_PATH; 
	pinmux += " " + pinName + function + "\n";
	if (system(pinmux.c_str()) != 0) return false;
	
	// set the direction
	std::ofstream direction;
	direction.open(path + "/direction");
	if (direction.is_open()) {
		if (_dir) {
			direction << "out";
		} else {
			direction << "in";
		}
		direction.close();
	} else return false;
	
	_init = true;
	return ((direction.good()) && this->writePin (_state));

}

bool Pin::setPort (int port) {
	if ((port != 8) && (port != 9)) return false;
	_port = port;
	_init = false;
	return true;
}	

bool Pin::setPin (int pin) {
	if (_port < 0) return false;
	_init = false;
	if (_port == 8) {
		if ((pin > 2) && (pin < 47)) {
			_pin = pin;
			return true;
		} 
	} else if (_port == 0) {
		if ((pin > 10) && (pin < 32)) {
			_pin = pin;
			return true;
		} else if ((pin > 40) && (pin < 43)) {
			_pin = pin;
			return true;
		}
	}
	return false;
}

bool Pin::writePin (bool val) {
	ofstream value;
	_state = val;
	value.open(path + "/value");
	if (value.is_open()) {
		if (_state) {
			value << "1";
		} else {
			value << "0";
		}
		value.close();
		return value.good();
	} else return false;
}
		
int Pin::get() {
	ifstream value;
	std::string line;
	value.open(path + "/value");
	if (value.is_open()) {
		if(getline(value, line)) {
			if (line[0] == '1') {
				_state = true;
				return 1;
			} else if (line[0] == '0') {
				_state = false;
				return 0;
			} else return -1;
		} else return -1;
	} else return -1;
}

bool Pin::pullUp () {
	function = "gpio_pu";
	if (_init) {
		std::string pinmux = CONFIG_PIN_PATH; 
		pinmux += " " + pinName + function + "\n";
		if (system(pinmux.c_str()) != 0) return false;
	}
	return true;
}

bool Pin::pullDown () {
	function = "gpio_pd";
	if (_init) {
		std::string pinmux = CONFIG_PIN_PATH; 
		pinmux += " " + pinName + function + "\n";
		if (system(pinmux.c_str()) != 0) return false;
	}
	return true;
}

bool Pin::floating () {
	function = "gpio";
	if (_init) {
		std::string pinmux = CONFIG_PIN_PATH; 
		pinmux += " " + pinName + function + "\n";
		if (system(pinmux.c_str()) != 0) return false;
	}
	return true;
}

int Pin::getGPIO (int port, int pin) {
	if (port == 8) {
		switch (pin) {
			case 3: 	return 38;
			case 4: 	return 39;
			case 5: 	return 34;
			case 6: 	return 35;
			case 7: 	return 66;
			case 8: 	return 67;
			case 9:		return 69;
			case 10:	return 68;
			case 11:	return 45;
			case 12:	return 44;
			case 13:	return 23;
			case 14:	return 26;
			case 15:	return 47;
			case 16:	return 46;
			case 17: 	return 27;
			case 18: 	return 65;
			case 19:	return 22;
			case 20:	return 63;
			case 21:	return 62;
			case 22:	return 37;
			case 23:	return 36;
			case 24:	return 33;
			case 25:	return 32;
			case 26:	return 61;
			case 27:	return 86;
			case 28: 	return 88;
			case 29:	return 87;
			case 30:	return 89;
			case 31:	return 10;
			case 32:	return 11;
			case 33:	return 9;
			case 34:	return 81;
			case 35:	return 8;
			case 36:	return 80;
			case 37:	return 78;
			case 38:	return 79;
			case 39:	return 76;
			case 40:	return 77;
			case 41: 	return 74;
			case 42:	return 75;
			case 43:	return 72;
			case 44:	return 73;
			case 45:	return 70;
			case 46:	return 71;
			default:	return -1;
		}
	} else if (port == 9) {
		switch (pin) {
			case 11: 	return 30;
			case 12:	return 60;
			case 13:	return 31;
			case 14:	return 50;
			case 15:	return 48;
			case 16:	return 51;
			case 17:	return 5;
			case 18:	return 4;
			case 21:	return 3;
			case 22:	return 2;
			case 23:	return 49;
			case 24: 	return 15;
			case 25:	return 117;
			case 26:	return 14;
			case 27:	return 115;
			case 28:	return 113;
			case 29:	return 111;
			case 30:	return 112;
			case 31:	return 110;
			case 41:	return 20;
			case 42:	return 7;
			default:	return -1;
		}
	} else return -1;
	return -1;
}
