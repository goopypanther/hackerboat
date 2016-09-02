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
#include "hal/drivers/i2c.hpp"
#include "hal/drivers/lsm303.hpp"

bool LSM303::begin() {
	bool result = true;
	ostringstream out = "";
	out << static_cast<char>(LSM303AccelRegistersEnum::LSM303_REGISTER_ACCEL_CTRL_REG1_A);
	out << static_cast<char>(LSM303AccelSpeedEnum::LSM303_ACCEL_100_HZ);
	if (_bus.openI2C(LSM303_ADDRESS_ACCEL)) {
		if (_bus.writeI2C(out) < 0) {
			_bus.closeI2C();
			return false;
		}
		_bus.closeI2C();
	} else return false;
	out = "";
	out << static_cast<char>(LSM303MagRegistersEnum::LSM303_REGISTER_MAG_MR_REG_M);
	out << 0x00;
	if (_bus.openI2C(LSM303_ADDRESS_MAG)) {
		if (_bus.writeI2C(out) < 0) {
			_bus.closeI2C();
			return false;
		}
		_bus.closeI2C();
	} else return false;
	return true;
}

bool LSM303::readMag ();
bool LSM303::readAccel ();
void LSM303::enableMagAutoRange(bool enable);							/**< Enable auto-ranging function (see data sheet). */
void LSM303::setMagGain(LSM303MagGainEnum gain);						/**< Set the magnetometer gain. */
map<char, double> LSM303::getMagData (void);							/**< Get the scaled magnetometer data. There will be three fields, named x, y, and z. */
map<char, double> LSM303::getAccelData (void);							/**< Get the scaled accelerometer data. Fields named as for magnetometer. */
void LSM303::setMagRegister(LSM303MagRegistersEnum reg, uint8_t val);	/**< Set an arbitrary register on the chip. */
uint8_t LSM303::getMagRegister(LSM303MagRegistersEnum reg);				/**< Read an arbitrary register on the chip. */
void LSM303::setAccelRegister(LSM303AccelRegistersEnum reg, uint8_t val);	/**< Set an arbitrary register on the chip. */
uint8_t LSM303::getAccelRegister(LSM303AccelRegistersEnum reg);			/**< Read an arbitrary register on the chip. */
bool LSM303::setMagOffset (map<char, int>);								/**< Set magnetometer offsets. */
bool LSM303::setAccelOffset (map<char, int>);							/**< Set accelerometer offsets. */
bool LSM303::setMagScale (map<char, double>);							/**< Set magnetometer scale. */
bool LSM303::setAccelScale (map<char, double>);							/**< Set accelerometer scale. */
