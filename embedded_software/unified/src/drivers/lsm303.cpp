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
#include <tuple>
#include "hal/drivers/lsm303.hpp"
extern "C" {
	#include "lsquaredc.h"
}

bool LSM303::setBus (uint8_t bus) {
	if (bus > 2) return false;
	_bus = bus;
	return true;
}

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
	int handle = i2c_open(_bus);
	bool result = false;
	if (handle >= 0) {
		std::vector<uint16_t> seq;
		seq.push_back(addr << 1);
		seq.push_back(reg);
		seq.push_back(val);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), NULL) >= 0) result = true;
	} 
	i2c_close(handle);
	return result;
}

int16_t LSM303::getReg (uint8_t addr, uint8_t reg) {
	int result = -1;
	int handle = i2c_open(_bus);
	if (handle >= 0) {
		std::vector<uint16_t> seq;
		uint8_t val;
		seq.push_back(addr << 1);
		seq.push_back(reg);
		seq.push_back(I2C_RESTART);
		seq.push_back((addr << 1)|1);
		seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), &val) >= 0) result = (int16_t)val;
	} 
	i2c_close(handle);
	return result;
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
	int handle = i2c_open(_bus);
	bool result = false;
	if (handle >= 0) {
		std::vector<uint8_t> buf(6);
		std::vector<uint16_t> seq;
		seq.push_back(LSM303_ADDRESS_MAG << 1);
		seq.push_back(static_cast<uint16_t>(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_OUT_X_H_M));
		seq.push_back(I2C_RESTART);
		seq.push_back((LSM303_ADDRESS_MAG << 1)|1);
		for (int i = 0; i < 6; i++) seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), buf.data()) >= 0) {
			get<0>(_magData) = (int16_t)((uint16_t)buf[1] | ((uint16_t)buf[0] << 8));
			get<2>(_magData) = (int16_t)((uint16_t)buf[3] | ((uint16_t)buf[2] << 8));
			get<1>(_magData) = (int16_t)((uint16_t)buf[5] | ((uint16_t)buf[4] << 8));
			result = true;
		}
	} 
	i2c_close(handle);
	return result;
}

bool LSM303::readAccel () {
	int handle = i2c_open(_bus);
	bool result = false;
	if (handle >= 0) {
		std::vector<uint8_t> buf(6);
		std::vector<uint16_t> seq;
		seq.push_back(LSM303_ADDRESS_ACCEL << 1);
		seq.push_back(static_cast<uint16_t>(LSM303AccelRegistersEnum::LSM303_REGISTER_ACCEL_OUT_X_L_A) + 0x80);
		seq.push_back(I2C_RESTART);
		seq.push_back((LSM303_ADDRESS_ACCEL << 1)|1);
		for (int i = 0; i < 6; i++) seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), buf.data()) >= 0) {
			get<0>(_accelData) = (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
			get<1>(_accelData) = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8));
			get<2>(_accelData) = (int16_t)((uint16_t)buf[4] | ((uint16_t)buf[5] << 8));
			result = true;
		}
	} 
	i2c_close(handle);
	return result;
}

bool LSM303::readTemp () {
	int handle = i2c_open(_bus);
	bool result = false;
	if (handle >= 0) {
		std::vector<uint8_t> buf(2);
		std::vector<uint16_t> seq;
		seq.push_back(LSM303_ADDRESS_MAG << 1);
		seq.push_back(static_cast<uint16_t>(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_TEMP_OUT_H_M) + 0x80);
		seq.push_back(I2C_RESTART);
		seq.push_back((LSM303_ADDRESS_MAG << 1)|1);
		for (int i = 0; i < 2; i++) seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), buf.data()) >= 0) {
			_tempData = (int)buf[1] + ((int)buf[0] * 0xff);
			result = true;
		}
	}
	i2c_close(handle);
	return result;
}

tuple<double, double, double> LSM303::getMagData (void) {
	tuple<double, double, double> result;
	get<0>(result) = (get<0>(_magData) + get<0>(_magOffset)) * get<0>(_magScale);
	get<1>(result) = (get<1>(_magData) + get<1>(_magOffset)) * get<1>(_magScale);
	get<2>(result) = (get<2>(_magData) + get<2>(_magOffset)) * get<2>(_magScale);
	return result;
}

tuple<double, double, double> LSM303::getAccelData (void) {
	tuple<double, double, double> result;
	get<0>(result) = (get<0>(_accelData) + get<0>(_accelOffset)) * get<0>(_accelScale);
	get<1>(result) = (get<1>(_accelData) + get<1>(_accelOffset)) * get<1>(_accelScale);
	get<2>(result) = (get<2>(_accelData) + get<2>(_accelOffset)) * get<2>(_accelScale);
	return result;
}

double LSM303::getTempData () {
	return (double)(_tempData + _tempOffset) * _tempScale;
}

bool LSM303::setMagOffset (tuple<int,int,int> offset) {
	_magOffset = offset;
	return true;
}

bool LSM303::setAccelOffset (tuple<int,int,int> offset) {
	_accelOffset = offset;
	return true;
}

bool LSM303::setMagScale (tuple<double,double,double> scale) {
	_magScale = scale;
	return true;
}

bool LSM303::setAccelScale (tuple<double,double,double> scale) {
	_accelScale = scale;
	return true;
}
