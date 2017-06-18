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
#include <chrono>
#include <vector>
#include <map>
#include <inttypes.h>
#include <iostream>

#include "hal/gpio.hpp"
#include <fcntl.h>  
#include <unistd.h>
#include <fstream>
#include "easylogging++.h"
#include "configuration.hpp"

// turn off debugging logging for this module
#ifndef NDEBUG
#define NDEBUG
#endif

using namespace std;

bool Pin::init () {
	if (_init) return true;
	_gpio = getGPIO(_port, _pin);
	if (_gpio < 0) {
		LOG(WARNING) << "Selected pin does not exist or is not available P" << _port << "_" << _pin;
		return false;
	}
	
	pinName = "P" + to_string(_port);
	pinName += "_" + to_string(_pin);
	
	// use config-pin to set the pinmux & direction
	std::string pinmux = Conf::get()->configPinPath(); 
	pinmux += " " + pinName + " " + function + "\n";
	LOG(DEBUG) << "Initializing pin with command " << pinmux;
	if (system(pinmux.c_str()) != 0) {
		LOG(WARNING) << "Failed to initialize pin with command " << pinmux;
		return false;
	}
	
	// assemble & test path
	path = "/sys/class/gpio/gpio" + to_string(_gpio);
	LOG(DEBUG) << "Pin path is " << path;
	if (access(path.c_str(), F_OK)) {      // check if the file exists & export if necessary
		string cmd = "sudo echo " + to_string(_gpio);
		cmd += " > /sys/class/gpio/export\n";
		if (system(cmd.c_str()) != 0) {
			LOG(WARNING) << "Failed to access pin at " << path << " with cmd " << cmd;
			return false;
		}
	} else {
		LOG(DEBUG) << "Pin already exported at " << path;
	}
		
	_init = true;
	return _init;

}

bool Pin::setPort (int port) {
	if ((port != 8) && (port != 9)) return false;
	_port = port;
	_init = false;
	VLOG(3) << "Set port to " << port;
	return true;
}	

bool Pin::setPin (int pin) {
	if (_port < 0) return false;
	_init = false;
	if (_port == 8) {
		if ((pin > 2) && (pin < 47)) {
			_pin = pin;
			VLOG(3) << "Set pin to " << pin << " on port 8";
			return true;
		} 
	} else if (_port == 0) {
		if ((pin > 10) && (pin < 32)) {
			_pin = pin;
			VLOG(3) << "Set pin to " << pin << " on port 9";
			return true;
		} else if ((pin > 40) && (pin < 43)) {
			_pin = pin;
			VLOG(3) << "Set pin to " << pin << " on port 9";
			return true;
		}
	}
	return false;
}

bool Pin::setDir (bool dir) {
	_dir = dir;
	if (!_init) {
		LOG(ERROR) << "Attempted to set the direction of uninitialized pin";
		return false;
	}
	ofstream direction;
	direction.open(path + "/direction");
	if (direction.is_open()) {
		if (_dir) {
			direction << "out";
		} else {
			direction << "in";
		}
		bool result = direction.good();
		direction.close();
		return result;
	} else {
		LOG(ERROR) << "Unable to set pin direction in " << path;
		return false;
	}
}

bool Pin::writePin (bool val) {
	ofstream value;
	_state = val;
	if (!_init) {
		LOG(ERROR) << "Attempted to write to an uninitialized pin";
		return false;
	}
	value.open(path + "/value");
	if (value.is_open()) {
		if (_state) {
			value << "1";
		} else {
			value << "0";
		}
		bool result = value.good();
		value.close();
		return result;
	} else {
		LOG(ERROR) << "Unable to write to pin" << path;
		return false;
	}
}
		
int Pin::get() {
	ifstream value;
	std::string line;
	int result = -1;
	if (!_init) {
		LOG(ERROR) << "Attempted to read from an uninitialized pin";
		return -1;
	}
	value.open(path + "/value");
	if (value.is_open()) {
		if(getline(value, line)) {
			if (line[0] == '1') {
				_state = true;
				result = 1;
			} else if (line[0] == '0') {
				_state = false;
				result = 0;
			} 
		} 
	}
	LOG_IF((result == -1), ERROR) << "Failed to open value for pin " << path;
	value.close();
	return result;
}

bool Pin::pullUp () {
	if (_dir) return false;
	function += "in_pu";
	if (_init) {
		std::string pinmux = Conf::get()->configPinPath(); 
		pinmux += " " + pinName + " " + function + "\n";
		if (system(pinmux.c_str()) != 0) {
			LOG(ERROR) << "Unable to set pull-up with " << pinmux;
			return false;
		}
	}
	return true;
}

bool Pin::pullDown () {
	if (_dir) return false;
	function = "in_pd";
	if (_init) {
		std::string pinmux = Conf::get()->configPinPath(); 
		pinmux += " " + pinName + " " + function + "\n";
		if (system(pinmux.c_str()) != 0) {
			LOG(ERROR) << "Unable to set pull-down with " << pinmux;
			return false;
		}
	}
	return true;
}

bool Pin::floating () {
	if (_dir) return false;
	function = "in";
	if (_init) {
		std::string pinmux = Conf::get()->configPinPath(); 
		pinmux += " " + pinName + function + "\n";
		if (system(pinmux.c_str()) != 0) {
			LOG(ERROR) << "Unable to set floating with " << pinmux;
			return false;
		}
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
