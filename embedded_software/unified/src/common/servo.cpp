/******************************************************************************
 * Hackerboat Beaglebone Servo module
 * hal/servo.cpp
 * This module manipulates the servo output
 * see the Hackerboat documentation for more details
 * Code is derived, in a general sense, from the Arduino Servo library
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <iostream>
#include <sstream>
#include <fcntl.h>  
#include <unistd.h>
#include <fstream>
#include <string>
#include <inttypes.h>
#include "hal/config.h"
#include "hal/servo.hpp"
#include "logs.hpp"

static LogError* mylog = LogError::instance();

bool Servo::attach (int port, int pin, int min, int max, int freq) {
	path = getServoPath(port, pin);
	if (path == "") return false;
	name = "P" + std::to_string(port);
	name += "_" + std::to_string(pin);
	
	_min = (min * 1000);		// convert milliseconds to microseconds
	_max = (max * 1000);		// convert milliseconds to microseconds
	_center = (_min + _max)/2;	// find the center point
	_freq = (1e9/freq);			// convert Hz into ns
	_val = _center;
	
	// call config-pin to turn things on
	// we assume that the correct udev rule has been invoked to fire up the pwm
	std::string pinmux = CONFIG_PIN_PATH;
	pinmux += " " + name + " pwm\n";
	if (system(pinmux.c_str()) != 0) {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to enable pinmux for " + name);
		mylog->close();
		return false;
	}
	
	// set the period, set the duty cycle, and enable
	if (!setFrequency() || !writeMicroseconds()) {
		detach();
		return false;
	}
	// write a 1 to the enable file to turn on pwm
	std::ofstream enable;
	enable.open(path + "/enable");
	if (enable.is_open()) {
		enable << "1";
		enable.close();
	} else {
		detach();
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to enable specified pwm channel or pin " + path + " " + name);
		mylog->close();
		return false;
	}
	attached = true;
	return true;
}

void Servo::detach () {
	_freq = 0;
	_val = 0;
	setFrequency();
	writeMicroseconds();
	std::ofstream enable;
	enable.open(path + "/enable");
	if (enable.is_open()) {
		enable << "0";
		enable.close();
	}
	std::string pinmux = CONFIG_PIN_PATH; 
	pinmux += " " + name + " default\n";
	if (system(pinmux.c_str()) != 0) {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to disable pinmux for " + name);
		mylog->close();
	}
	attached = false;
}

bool Servo::write (double value) {
	if (!attached) return false;
	if (value > 100.0) return false;
	if (value < -100.0) return false;
	
	long span = _max - _min;	// this should probably be pre-calculated
	// We divide span by 200.0 to get the number of nanosecconds for each count of the input span
	// We add 100 to the input value so it runs from {0 - 200} rather than {-100 - 100}
	_val = floor((value+100)*(span/200.0)) + _min;
	return writeMicroseconds();
}

bool Servo::writeMicroseconds (unsigned long value) {
	if (value < _min) return false;
	if (value > _max) return false;
	_val = value;
	return true;
}

bool Servo::setFrequency (unsigned long freq) {
	_freq = (1e9/freq);
	if (_max > _freq) _max = _freq;
	if (_min > _freq) _min = _freq;
	return setFrequency();
}

bool Servo::writeMicroseconds () {
	std::ofstream duty;
	duty.open(path + "/duty_cycle");
	if (duty.is_open()) {
		duty << _val;
		duty.close();
	} else {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to open duty_cycle for " + path + " " + name);
		mylog->close();
		return false;
	}
	if (duty.bad()) {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to write duty_cycle for " + path + " " + name);
		mylog->close();
		return false;
	}
	return true;
}

bool Servo::setFrequency () {
	std::ofstream freq;
	freq.open(path + "/period");
	if (freq.is_open()) {
		freq << _freq;
		freq.close();
	} else {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to open period for " + path + " " + name);
		mylog->close();
		return false;
	}
	if (freq.bad()) {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("SERVO", "Unable to write period for " + path + " " + name);
		mylog->close();
		return false;
	}
	return true;
}

bool Servo::setMax (unsigned long max) {
	max *= 1000;
	if (max > _freq) return false;
	_max = max;
	return false;
}

bool Servo::setMin (unsigned long min) {
	min *= 1000;
	if (min > _max) return false;
	_min = min;
	return false;
}

double Servo::read() {
	int span = _max - _min;
	int posn = _val - _min;
	return ((double)posn*(200.0/(double)span)) - 100.0;
}

unsigned long Servo::readMicroseconds() {
	return _val/1000;
}

std::string Servo::getServoPath (int port, int pin) {
	if (port == 9) {
		switch (pin) {
			case 14:	return "/sys/class/pwm/pwmchip4/pwm0";
			case 16:	return "/sys/class/pwm/pwmchip4/pwm1";
			case 21:	return "/sys/class/pwm/pwmchip0/pwm1";
			case 22:	return "/sys/class/pwm/pwmchip0/pwm0";
			case 28:	return "/sys/class/pwm/pwmchip3/pwm0"; 
			case 29:	return "/sys/class/pwm/pwmchip0/pwm1";
			case 31:	return "/sys/class/pwm/pwmchip0/pwm0";
			case 42:	return "/sys/class/pwm/pwmchip2/pwm0";
			default: 	return "";
		}
	} else if (port == 8) {
		switch (pin) {
			case 13:	return "/sys/class/pwm/pwmchip6/pwm1";
			case 19:	return "/sys/class/pwm/pwmchip6/pwm0";
			case 34:	return "/sys/class/pwm/pwmchip4/pwm1";
			case 36:	return "/sys/class/pwm/pwmchip4/pwm0";
			case 45:	return "/sys/class/pwm/pwmchip6/pwm0";
			case 46:	return "/sys/class/pwm/pwmchip6/pwm1";
			default: 	return "";
		}
	} else return "";
	return "";
}
