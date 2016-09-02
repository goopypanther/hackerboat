/******************************************************************************
 * Hackerboat Beaglebone LSM303 module
 * hal/drivers/LSM303.cpp
 * This module provides an interface to the LSM303 accelerometer & compass 
 * see the Hackerboat documentation for more details
 * Code is derived from the Adafruit LSM303 and Adafruit Sensor libraries
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <errno.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "hal/config.h"
#include <map>
#include <vector>
#include "hal/drivers/i2c.hpp"
#include "hal/drivers/lsm303.hpp"

bool LSM303::begin() {
	bool result = true;
	result &= setAccelRegister(LSM303AccelRegistersEnum::LSM303_REGISTER_ACCEL_CTRL_REG1_A, 
								static_cast<unsigned char>(LSM303AccelSpeedEnum::LSM303_ACCEL_100_HZ));
	result &= setMagRegister(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_MR_REG_M, 0x00);
	result &= (getAccelRegister(LSM303AccelRegistersEnum::LSM303_REGISTER_ACCEL_CTRL_REG1_A) ==
				static_cast<unsigned char>(LSM303AccelSpeedEnum::LSM303_ACCEL_100_HZ));
	result &= (getMagRegister(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_MR_REG_M) == 0x00);
	return result;
}

bool LSM303::setReg (uint8_t addr, uint8_t reg, uint8_t val) {
	std::vector<uint8_t> out;
	out.push_back(reg);
	out.push_back(val); 
	if (_bus.openI2C(addr)) {
		if (_bus.writeI2C(out) < 0) {
			_bus.closeI2C();
			return false;
		}
		_bus.closeI2C();
	} else return false;
	return true;
}

unsigned char LSM303::getReg (uint8_t addr, uint8_t reg) {
	std::vector<uint8_t> out, result;
	out.push_back(reg);
	if (_bus.openI2C(addr)) {
		if (_bus.writeI2C(out) < 0) {
			_bus.closeI2C();
			return 0x00;
		} else if (_bus.readI2C(result, 1, 1) != 1) {
			_bus.closeI2C();
			return 0x00;
		}
	} else return 0x00;
	_bus.closeI2C();
	return result[0];
}

bool LSM303::setMagRegister(LSM303MagRegistersEnum reg, unsigned char val) {
	return setReg(LSM303_ADDRESS_MAG, static_cast<uint8_t>(reg), (uint8_t)val);
}

unsigned char LSM303::getMagRegister(LSM303MagRegistersEnum reg) {
	return getReg(LSM303_ADDRESS_MAG, static_cast<uint8_t>(reg));
}

bool LSM303::setAccelRegister(LSM303AccelRegistersEnum reg, unsigned char val) {
	return setReg(LSM303_ADDRESS_ACCEL, static_cast<uint8_t>(reg), (uint8_t)val);
}

unsigned char LSM303::getAccelRegister(LSM303AccelRegistersEnum reg) {
	return getReg(LSM303_ADDRESS_ACCEL, static_cast<uint8_t>(reg));
}

bool LSM303::readMag () {
	std::vector<uint8_t> buf(6), get;
	get.push_back(static_cast<uint8_t>(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_OUT_X_H_M));
	if (_bus.openI2C(LSM303_ADDRESS_MAG)) {
		if (_bus.writeI2C(get) < 0) {
			_bus.closeI2C();
			return false;
		} else if (_bus.readI2C(buf, 6, 6) != 6) {
			_bus.closeI2C();
			return false;
		} else {
			_magData['x'] = (int)buf[1] + ((int)buf[0] * 0xff);
			_magData['z'] = (int)buf[3] + ((int)buf[2] * 0xff);
			_magData['y'] = (int)buf[5] + ((int)buf[4] * 0xff);
		}
	} else return false;
	_bus.closeI2C();
	return true;
}

bool LSM303::readAccel () {
	std::vector<uint8_t> buf(6), get;
	get.push_back(static_cast<uint8_t>(LSM303AccelRegistersEnum::LSM303_REGISTER_ACCEL_OUT_X_L_A));
	if (_bus.openI2C(LSM303_ADDRESS_ACCEL)) {
		if (_bus.writeI2C(get) < 0) {
			_bus.closeI2C();
			return false;
		} else if (_bus.readI2C(buf, 6, 6) != 6) {
			_bus.closeI2C();
			return false;
		} else {
			_accelData['x'] = (int)buf[0] + ((int)buf[1] * 0xff);
			_accelData['y'] = (int)buf[2] + ((int)buf[3] * 0xff);
			_accelData['z'] = (int)buf[4] + ((int)buf[5] * 0xff);
		}
	} else return false;
	_bus.closeI2C();
	return true;	
}

bool LSM303::readTemp () {
	std::vector<uint8_t> buf(2), get;
	get.push_back(static_cast<uint8_t>(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_TEMP_OUT_H_M));
	if (_bus.openI2C(LSM303_ADDRESS_MAG)) {
		if (_bus.writeI2C(get) < 0) {
			_bus.closeI2C();
			return false;
		} else if (_bus.readI2C(buf, 2, 2) != 2) {
			_bus.closeI2C();
			return false;
		} else {
			_tempData = (int)buf[1] + ((int)buf[0] * 0xff);
		}
	} else return false;
	_bus.closeI2C();
	return true;		
}

map<char, double> LSM303::getMagData (void) {
	map<char, double> result;
	for (auto& item : _magData) {
		char c = item.first;
		result[c] =  (double)(item.second + _magOffset[c]) * _magScale[c];
	}
	return result;
}

map<char, double> LSM303::getAccelData (void) {
	map<char, double> result;
	for (auto& item : _accelData) { 
		char c = item.first;
		result[c] =  (double)(item.second + _accelOffset[c]) * _accelScale[c];
	}
	return result;
}

double LSM303::getTempData () {
	return (double)(_tempData + _tempOffset) * _tempScale;
}

bool LSM303::setMagOffset (map<char, int> offset) {
	int count = 0;
	for (auto& item : _magOffset) {
		char c = item.first;
		if (offset.find(c) != offset.end()) {
			item.second = offset[c];
			count++;
		}
	}
	return (count) ? true : false;
}

bool LSM303::setAccelOffset (map<char, int> offset) {
	int count = 0;
	for (auto& item : _accelOffset) {
		char c = item.first;
		if (offset.find(c) != offset.end()) {
			item.second = offset[c];
			count++;
		}
	}
	return (count) ? true : false;
}

bool LSM303::setMagScale (map<char, double> scale) {
	int count = 0;
	for (auto& item : _magScale) {
		char c = item.first;
		if (scale.find(c) != scale.end()) {
			item.second = scale[c];
			count++;
		}
	}
	return (count) ? true : false;	
}

bool LSM303::setAccelScale (map<char, double> scale) {
	int count = 0;
	for (auto& item : _accelScale) {
		char c = item.first;
		if (scale.find(c) != scale.end()) {
			item.second = scale[c];
			count++;
		}
	}
	return (count) ? true : false;	 
}
