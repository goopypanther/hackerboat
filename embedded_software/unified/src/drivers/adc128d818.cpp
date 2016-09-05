/******************************************************************************
 * Hackerboat adc128d818 driver module
 * hal/adc128d818.cpp
 * This module reads analog data from ADC128D818 devices
 * Based on https://github.com/bryanduxbury/adc128d818_driver
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <errno.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include "logs.hpp"
#include "hal/config.h"
#include "hal/drivers/i2c.hpp"
#include "hal/drivers/adc128d818.hpp"

#define CONFIG_REG    		((uint8_t)0x00)
#define CONV_RATE_REG 		((uint8_t)0x07)
#define CHANNEL_DISABLE_REG ((uint8_t)0x08)
#define ADV_CONFIG_REG 		((uint8_t)0x0B)
#define BUSY_STATUS_REG 	((uint8_t)0x0C)

#define READ_REG_BASE 		((uint8_t)0x20)
#define LIMIT_REG_BASE 		((uint8_t)0x2A)

#define TEMP_REGISTER 		((uint8_t)0x27)

#define ADC_COUNTS			(4096)

#define START_BIT 0
#define INIT_BIT 7

#define EXT_REF_ENABLE 0
#define MODE_SELECT_1 1
#define MODE_SELECT_2 2

#define BUSY_BIT 0
#define NOT_READY_BIT 1

static LogError* mylog = LogError::instance();

ADC128D818::ADC128D818(uint8_t address, I2CBus bus) :
	addr(address), _bus(bus), disabled_mask(0), ref_v(ADC128D818_INTERNAL_REF),
	ref_mode(reference_mode_t::INTERNAL_REF), op_mode(operation_mode_t::SINGLE_ENDED),
	conv_mode(conv_mode_t::CONTINUOUS) {}

void ADC128D818::setReference(double ref_voltage) {
	ref_v = ref_voltage;
}

void ADC128D818::setReferenceMode(reference_mode_t mode) {
	ref_mode = mode;
}

void ADC128D818::setOperationMode(operation_mode_t mode) {
	op_mode = mode;
}

void ADC128D818::setConversionMode(conv_mode_t mode) {
	conv_mode = mode;
}

void ADC128D818::setDisabledMask(uint8_t disabled_mask) {
	this->disabled_mask = disabled_mask;
}

bool ADC128D818::writeByteRegister(uint8_t reg, uint8_t data) {
	std::vector<uint8_t> out {2};
	out[0] = reg;
	out[1] = data;
	if ((_bus.openI2C(addr)) && (_bus.writeI2C(out))) {
		_bus.closeI2C();
		return true;
	} else {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("ADC128D818", "Failed to write to ADC register " + reg);
		mylog->close();
		_bus.closeI2C();
		return false;
	}
	return false;
}

bool ADC128D818::readByteRegister(uint8_t reg, uint8_t& data) {
	std::vector<uint8_t> io {1, reg};
	// We take advantage of short-circuiting here. Order is guaranteed, per standard.
	if ((_bus.openI2C(addr)) && (_bus.writeI2C(io)) && (_bus.readI2C(io, 1, 1))) {
		_bus.closeI2C();
		data = io[0];	// we use the one vector both coming and going
		return true;
	} else {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("ADC128D818", "Failed to connect and/or write to and/or read from ADC register " + reg);
		mylog->close();
		_bus.closeI2C();
		return false;
	}
	return false;
}

bool ADC128D818::begin() {
	uint8_t data { 0xff };				// We initialize the two bytes of this vector to all ones so the while loop fires onces
	uint8_t reg { BUSY_STATUS_REG };	// This is the first register up, so we might as well start here. 
	int count = 0;
	while (data && count < ADC_START_COUNT) { 	// check the ready & busy bits. Once they're all zero, we can proceed 
		readByteRegister(reg, data);
		count++;
		std::this_thread::sleep_for(ADC_START_PERIOD);
	}
	_bus.closeI2C();
	if (count >= BUSY_STATUS_REG) {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("ADC128D818", "Failed to connect to ADC and/or ADC was stuck in startup or busy mode.");
		mylog->close();
		return false;
	}
	
	// Shut down the ADC and restore defaults.
	if (!writeByteRegister(CONFIG_REG, 0x80)) return false;
	
	// Program the advanced configuration register
	if (!writeByteRegister(ADV_CONFIG_REG, ((uint8_t)ref_mode | ((uint8_t)op_mode << 1)))) return false;
	
	// Program conversion rate register
	if (!writeByteRegister(CONV_RATE_REG, (uint8_t)conv_mode)) return false;
	
	// Program enabled channels
	if (!writeByteRegister(CHANNEL_DISABLE_REG, disabled_mask)) return false;
	
	// Activate ADC
	if (!writeByteRegister(CONFIG_REG, 0x01)) return false;
	
	return true;
}

int16_t ADC128D818::read(uint8_t channel) {
	if (disabled_mask & (((uint8_t)1)<<channel)) return -1;	// check if this channel has been disabled and bail if it has
	std::vector<uint8_t> reg {1, (uint8_t)(READ_REG_BASE + channel)};
	std::vector<uint8_t> data {2};		
	if ((_bus.openI2C(addr)) && (_bus.writeI2C(reg)) && (_bus.readI2C(data, 2, 2))) {
		_bus.closeI2C();
		return (data[1] + (((uint16_t)data[0]) << 8));
	} else {
		mylog->open(HARDWARE_LOGFILE);
		mylog->write("ADC128D818", "Failed to connect to and/or read from ADC register " + reg[0]);
		mylog->close();
		_bus.closeI2C();
		return -1;
	}
	return -1;
}

vector<int16_t> ADC128D818::readAll (void) {
	std::vector<int16_t> data {8};
	for (int i = 0; i < 7; i++) {
		data[i] = this->read(i);
	}
	return data;
}

double ADC128D818::readScaled(uint8_t channel) {
	int16_t data = this->read(channel);
	if (data >= 0) {
		return ((double)(this->read(channel)) * (ref_v/ADC_COUNTS));
	} else return NAN;

}

vector<double> ADC128D818::readScaled (void) {
	std::vector<double> data {8};
	for (int i = 0; i < 7; i++) {
		data[i] = this->readScaled(i);
	}
	return data;
}

double ADC128D818::readTemperatureScaled() {
	if (op_mode == operation_mode_t::SINGLE_ENDED_WITH_TEMP) {
		uint16_t raw = this->read(7);
		if (raw & 0x100) {
			return -(double)(512 - raw) / 2;
		} else {
			return (double)raw / 2;
		}
	} else return NAN;
}