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
#include <fcntl.h>  
#include <unistd.h>
#include <fstream>
#include <string>
#include <inttypes.h>

#include "hal/servo.hpp"
#include "easylogging++.h"
#include "configuration.hpp"

static const std::string basepath = "/sys/class/pwm/pwmchip";

using namespace std;

Servo::Servo() : path(""), pinname("PPPP"), majornum(-1), minornum(-1), attached(false) {}

bool Servo::attach (int port, int pin, long min, long max, long freq) {
	path = getServoPath(port, pin);
	LOG(DEBUG) << "Attaching servo to " << path;
	if (path == "") return false;
	pinname = "P";
	pinname += to_string(port);
	pinname += "_" + to_string(pin);
	
	_min = (min * 1000);		// convert microsecond to nanoseconds
	_max = (max * 1000);		// convert microsecond to nanoseconds
	_center = (_min + _max)/2;	// find the center point
	_freq = (1e9/freq);			// convert Hz into ns
	_val = _center;
	
	// Turn on the PWM subsystem, set permissions correctly, and check that we've got access
	// It includes some very rude hacks using sudo, chown, and chmod to get the permissions right
	// because I can't seem to get the motherfucking udev rule to do the right thing in any sort of
	// consistent fashion.
	std::string chmodcmd = "sudo chown -R root:gpio /sys/class/pwm; sudo chmod -R 0770 /sys/class/pwm";
	system(chmodcmd.c_str());
	chmodcmd = "sudo chown -R root:gpio /sys/devices/platform/ocp/4????000.epwmss/*; sudo chmod -R 0770 /sys/devices/platform/ocp/4????000.epwmss/*";
	system(chmodcmd.c_str());
	std::string exportcmd = "echo " + std::to_string(minornum) + " > ";
	exportcmd += basepath + std::to_string(majornum) + "/export";
	// check if the export exists before attempting to export it to avoid errors
	if (access(path.c_str(), F_OK)) {
		if (system(exportcmd.c_str()) != 0)  {
			LOG(ERROR) << "Unable to export pwm channel for " << path;
			return false;
		}	
	}
	system(chmodcmd.c_str());
	if (access(path.c_str(), W_OK)) {
		LOG(ERROR) << "Unable to write to pwm channel for " << path;
		return false;
	}
	
	// call config-pin to turn things on
	// we assume that the correct udev rule has been invoked to fire up the pwm
	std::string pinmux = Conf::get()->configPinPath();
	pinmux += " " + pinname + " pwm\n";
	if (system(pinmux.c_str()) != 0) {
		LOG(ERROR) << "Unable to enable pinmux " << pinmux;
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
		LOG(ERROR) << "Unable to enable specified pwm channel [" << path << "] or pin [" << pinname;
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
	std::string pinmux = Conf::get()->configPinPath(); 
	pinmux += " " + pinname + " default\n";
	if (system(pinmux.c_str()) != 0) {
		LOG(ERROR) << "Unable to disable pinmux for " << pinname;
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
	return writeMicroseconds();
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
		duty << to_string(_val);
		duty.close();
	} else {
		LOG(ERROR) << "Unable to open duty_cycle for " << path << " " << pinname;
		return false;
	}
	if (duty.bad()) {
		LOG(ERROR) << "Unable to open duty_cycle for " << path << " " << pinname;
		return false;
	}
	return true;
}

bool Servo::setFrequency () {
	std::ofstream freq;
	freq.open(path + "/period");
	if (freq.is_open()) {
		freq << to_string(_freq);
		freq.close();
	} else {
		LOG(ERROR) << "Unable to write period for " << path << " " << pinname;
		return false;
	}
	if (freq.bad()) {
		LOG(ERROR) << "Unable to write period for " << path << " " << pinname;
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
			case 14:
				majornum = 4;
				minornum = 0;
				break;
			case 16:
				majornum = 4;
				minornum = 1;
				break;
			case 21:
				majornum = 0;
				minornum = 1;
				break;
			case 22:
				majornum = 0;
				minornum = 0;
				break;
			case 28:
				majornum = 3;
				minornum = 0;
				break;
			case 29:
				majornum = 0;
				minornum = 1;
				break;
			case 31:
				majornum = 0;
				minornum = 0;
				break;
			case 42:
				majornum = 2;
				minornum = 0;
				break;
			default: 	return "";
		}
	} else if (port == 8) {
		switch (pin) {
			case 13:
				majornum = 6;
				minornum = 1;
				break;
			case 19:
				majornum = 6;
				minornum = 0;
				break;
			case 34:
				majornum = 4;
				minornum = 1;
				break;
			case 36:
				majornum = 4;
				minornum = 0;
				break;
			case 45:
				majornum = 6;
				minornum = 0;
				break;
			case 46:
				majornum = 6;
				minornum = 1;
				break;
			default: 	return "";
		}
	} else return "";
	string result = basepath + to_string(majornum) + "/pwm" + to_string(minornum);
	return result;
}
