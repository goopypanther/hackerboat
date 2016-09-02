/******************************************************************************
 * Hackerboat Beaglebone I2C module
 * hal/drivers/i2c.hpp
 * This module provides an interface for I2C
 * see the Hackerboat documentation for more details
 * Code is derived from the Arduino Wire library and Derek Molloy's I2C tutorial
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "hal/config.h"
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "hal/drivers/i2c.hpp"
					
I2CDriver::I2CDriver (I2CBus bus) {
		this->setBus(bus);
}

bool I2CDriver::setBus (I2CBus bus) {
	switch (bus) {
		case I2CBus::BUS_0:
			path = "/dev/i2c-0"; 
			break;
		case I2CBus::BUS_1:
			path = "/dev/i2c-1"; 
			break;
		case I2CBus::BUS_2:
			path = "/dev/i2c-2"; 
			break;
		default:
			return false;
			break;
	}
	return true;
}

bool I2CDriver::openI2C (uint8_t address) {
	if ((file = open(path.c_str(), O_RDWR)) < 0) {
		std::cerr << "Failed to open I2C bus." << std::endl;
		return false;
	}
	if (ioctl(file, I2C_SLAVE, address) < 0){
		std::cerr << "I2C_SLAVE address " << address << " failed..." << std::endl;
		return false;
    }
	return true;
}

int I2CDriver::writeI2C (std::vector<uint8_t> output) {
	if (write(file, output.data(), output.size()) != (long int)output.size()) {	// the cast is to make this comparison not throw a warning
		std::cerr << "Failed to write data to I2C" << std::endl;
		return false;
	} 
	return true;
}

int I2CDriver::readI2C (std::vector<uint8_t>& input, int maxBytes, int minBytes) {
	std::vector<uint8_t> buf (maxBytes);
	int bytesRead = read(file, buf.data(), maxBytes);
	if (bytesRead < 0) {
		std::cerr << "I2C read failed" << std::endl;
		return bytesRead;
	} else if (bytesRead <= minBytes) {
		std::cerr << "I2C read did not read at least " << minBytes << " bytes." << std::endl;
		return 0;
	} 
	input = buf;
	return bytesRead;
	
}

bool I2CDriver::closeI2C() {
	close(file);
	file = -1;
	return true;
}

bool I2CDriver::isOpen () {
	return (file > 0);
}