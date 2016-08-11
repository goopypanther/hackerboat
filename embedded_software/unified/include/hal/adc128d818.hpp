/******************************************************************************
 * Hackerboat adc128d818 driver module
 * hal/adc128d818.hpp
 * This module reads analog data from ADC128D818 devices
 * Based on https://github.com/bryanduxbury/adc128d818_driver
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ADC128D818_H
#define ADC128D818_H

#include <errno.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include "hal/config.h"
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

enum reference_mode_t {
  INTERNAL_REF = 0, EXTERNAL_REF = 1
};

enum conv_mode_t {
  LOW_POWER, CONTINUOUS, ONE_SHOT
};

enum operation_mode_t {
  SINGLE_ENDED_WITH_TEMP = 0,
  SINGLE_ENDED = 1, 
  DIFFERENTIAL = 2,
  MIXED = 3
};

class ADC128D818 {
	public:
		ADC128D818(uint8_t address, std::string devpath);
  
		void setReference(double ref_voltage);
		void setReferenceMode(reference_mode_t mode);

		void setOperationMode(operation_mode_t mode);

		void setDisabledMask(uint8_t disabled_mask);

		void setConversionMode(conv_mode_t mode);

		void begin();

		uint16_t read(uint8_t channel);
		
		vector<uint16_t> readAll (void);

		double readConverted(uint8_t channel);
		
		vector<double> readAllConverted (void);

		double readTemperatureConverted();

	private:
		std::string path;
		int devhandle;
	
		uint8_t addr;
		uint8_t disabled_mask;
		double ref_v;

		reference_mode_t ref_mode;
		operation_mode_t op_mode;
		conv_mode_t conv_mode;

		void setRegisterAddress(uint8_t reg_addr);
		void setRegister(uint8_t reg_addr, uint8_t value);
		uint8_t readCurrentRegister8();
};


#endif
