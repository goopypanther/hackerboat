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

#include "hal/drivers/adc128d818.hpp"
#include "configuration.hpp"

// Names ending in _r are register addresses
// Names ending in _b are bit numbers
static const uint8_t config_r			= 0x00;
static const uint8_t conversionRate_r	= 0x07;
static const uint8_t chanDisable_r 		= 0x08;
static const uint8_t advancedConfig_r 	= 0x0B;
static const uint8_t busyStatus_r 		= 0x0C;
static const uint8_t readBase_r			= 0x20;
static const uint8_t limitBase_r		= 0x2A;
static const uint8_t temperature_r		= 0x27;
static const unsigned int adcCounts 	= 4096;
static const uint8_t start_b  			= 0x00;
static const uint8_t init_b  			= 0x07;
static const uint8_t extRefEnable_b		= 0x00;
static const uint8_t modeSel1_b			= 0x01;
static const uint8_t modeSel2_b			= 0x02;
static const uint8_t busy_b 			= 0x00;
static const uint8_t notReady_b 		= 0x01;	
static const unsigned int adcStartCount = 30;
static const sysdur adcStartPeriod 		= 500us;

//#define CONFIG_REG    		((uint8_t)0x00)
//#define CONV_RATE_REG 		((uint8_t)0x07)
//#define CHANNEL_DISABLE_REG ((uint8_t)0x08)
//#define ADV_CONFIG_REG 		((uint8_t)0x0B)
//#define BUSY_STATUS_REG 	((uint8_t)0x0C)

//#define READ_REG_BASE 		((uint8_t)0x20)
//#define LIMIT_REG_BASE 		((uint8_t)0x2A)

//#define TEMP_REGISTER 		((uint8_t)0x27)

//#define ADC_COUNTS			(4096)

//#define START_BIT 0
//#define INIT_BIT 7

//#define EXT_REF_ENABLE 0
//#define MODE_SELECT_1 1
//#define MODE_SELECT_2 2

//#define BUSY_BIT 0
//#define NOT_READY_BIT 1

using namespace std;

ADC128D818::ADC128D818(uint8_t address, int bus) :
	addr(address), _bus(bus), disabled_mask(0), ref_v(internalRefVolt),
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
	int handle = i2c_open(_bus);
	bool result = false;
	if (handle >= 0) {
		std::vector<uint16_t> seq;
		seq.push_back(addr << 1);
		seq.push_back(reg);
		seq.push_back(data);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), NULL) >= 0) result = true;
	} 
	i2c_close(handle);
	return result;
}

bool ADC128D818::readByteRegister(uint8_t reg, uint8_t& data) {
	bool result = false;
	int handle = i2c_open(_bus);
	if (handle >= 0) {
		std::vector<uint16_t> seq;
		seq.push_back(addr << 1);
		seq.push_back(reg);
		seq.push_back(I2C_RESTART);
		seq.push_back((addr << 1)|1);
		seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), &data) >= 0) result = true;
	} 
	i2c_close(handle);
	return result;
}

bool ADC128D818::begin() {
	uint8_t data { 0xff };				// We initialize this value to all ones so the while loop fires onces
	uint8_t reg { busyStatus_r };	// This is the first register up, so we might as well start here. 
	unsigned int count = 0;
	while ((data & 0x02) && (count < adcStartCount)) { 	// Check that the chip has started. 
		readByteRegister(reg, data);
		count++;
		std::this_thread::sleep_for(adcStartPeriod);
	}
	if (count >= busyStatus_r) {
		//mylog->write("ADC128D818", "Failed to connect to ADC and/or ADC was stuck in startup or busy mode.");
		return false;
	}
	
	// Shut down the ADC and restore defaults.
	if (!writeByteRegister(config_r, 0x80)) return false;
	
	// Program the advanced configuration register
	if (!writeByteRegister(advancedConfig_r, ((uint8_t)ref_mode | ((uint8_t)op_mode << 1)))) return false;
	
	// Program conversion rate register
	if (!writeByteRegister(conversionRate_r, (uint8_t)conv_mode)) return false;
	
	// Program enabled channels
	if (!writeByteRegister(chanDisable_r, disabled_mask)) return false;
	
	// Activate ADC
	if (!writeByteRegister(config_r, 0x01)) return false;
	
	return true;
}

int16_t ADC128D818::read(uint8_t channel) {
	if (disabled_mask & (((uint8_t)1)<<channel)) return -1;	// check if this channel has been disabled and bail if it has
	int result = -1;
	int handle = i2c_open(_bus);
	std::vector<uint16_t> seq;
	uint8_t buf[4];
	if (handle >= 0) {
		seq.push_back(addr << 1);
		seq.push_back(readBase_r + channel);
		seq.push_back(I2C_RESTART);
		seq.push_back(((addr << 1)|1));
		seq.push_back(I2C_READ);
		seq.push_back(I2C_READ);
		if (i2c_send_sequence(handle, seq.data(), seq.size(), buf) >= 0) {
			result = (int)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
		}
	} else {
		//mylog->write("ADC128D818", "Failed to connect to and/or read from ADC register " + (READ_REG_BASE + channel));
		result = -1;
	}
	i2c_close(handle);
	return result;
}

vector<int> ADC128D818::readAll (void) {
	std::vector<int> data;
	for (int i = 0; i < 7; i++) {
		data.push_back(this->read(i));
	}
	return data;
}

double ADC128D818::readScaled(uint8_t channel) {
	int16_t data = this->read(channel);
	if (data >= 0) {
		return ((double)(this->read(channel)) * (ref_v/adcCounts));
	} else return NAN;

}

vector<double> ADC128D818::readScaled (void) {
	std::vector<double> data;
	for (int i = 0; i < 7; i++) {
		data.push_back(this->readScaled(i));
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
