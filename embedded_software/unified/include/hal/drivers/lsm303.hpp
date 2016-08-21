/******************************************************************************
 * Hackerboat Beaglebone LSM303 module
 * hal/lsm303.hpp
 * This module provides an interface to the LSM303 accelerometer & compass 
 * see the Hackerboat documentation for more details
 * Code is derived from the Adafruit LSM303 and Adafruit Sensor libraries
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef LSM303_H
#define LSM303_H

#include <errno.h>
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
#include <map>
#include "hal/drivers/i2c.hpp"


/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define LSM303_ADDRESS_ACCEL          (0x32 >> 1)         // 0011001x
    #define LSM303_ADDRESS_MAG            (0x3C >> 1)         // 0011110x
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    enum class lsm303AccelRegisters_t
    {                                                     // DEFAULT    TYPE
      LSM303_REGISTER_ACCEL_CTRL_REG1_A         = 0x20,   // 00000111   rw
      LSM303_REGISTER_ACCEL_CTRL_REG2_A         = 0x21,   // 00000000   rw
      LSM303_REGISTER_ACCEL_CTRL_REG3_A         = 0x22,   // 00000000   rw
      LSM303_REGISTER_ACCEL_CTRL_REG4_A         = 0x23,   // 00000000   rw
      LSM303_REGISTER_ACCEL_CTRL_REG5_A         = 0x24,   // 00000000   rw
      LSM303_REGISTER_ACCEL_CTRL_REG6_A         = 0x25,   // 00000000   rw
      LSM303_REGISTER_ACCEL_REFERENCE_A         = 0x26,   // 00000000   r
      LSM303_REGISTER_ACCEL_STATUS_REG_A        = 0x27,   // 00000000   r
      LSM303_REGISTER_ACCEL_OUT_X_L_A           = 0x28,
      LSM303_REGISTER_ACCEL_OUT_X_H_A           = 0x29,
      LSM303_REGISTER_ACCEL_OUT_Y_L_A           = 0x2A,
      LSM303_REGISTER_ACCEL_OUT_Y_H_A           = 0x2B,
      LSM303_REGISTER_ACCEL_OUT_Z_L_A           = 0x2C,
      LSM303_REGISTER_ACCEL_OUT_Z_H_A           = 0x2D,
      LSM303_REGISTER_ACCEL_FIFO_CTRL_REG_A     = 0x2E,
      LSM303_REGISTER_ACCEL_FIFO_SRC_REG_A      = 0x2F,
      LSM303_REGISTER_ACCEL_INT1_CFG_A          = 0x30,
      LSM303_REGISTER_ACCEL_INT1_SOURCE_A       = 0x31,
      LSM303_REGISTER_ACCEL_INT1_THS_A          = 0x32,
      LSM303_REGISTER_ACCEL_INT1_DURATION_A     = 0x33,
      LSM303_REGISTER_ACCEL_INT2_CFG_A          = 0x34,
      LSM303_REGISTER_ACCEL_INT2_SOURCE_A       = 0x35,
      LSM303_REGISTER_ACCEL_INT2_THS_A          = 0x36,
      LSM303_REGISTER_ACCEL_INT2_DURATION_A     = 0x37,
      LSM303_REGISTER_ACCEL_CLICK_CFG_A         = 0x38,
      LSM303_REGISTER_ACCEL_CLICK_SRC_A         = 0x39,
      LSM303_REGISTER_ACCEL_CLICK_THS_A         = 0x3A,
      LSM303_REGISTER_ACCEL_TIME_LIMIT_A        = 0x3B,
      LSM303_REGISTER_ACCEL_TIME_LATENCY_A      = 0x3C,
      LSM303_REGISTER_ACCEL_TIME_WINDOW_A       = 0x3D
    };
    
    enum class lsm303MagRegisters_t
    {
      LSM303_REGISTER_MAG_CRA_REG_M             = 0x00,
      LSM303_REGISTER_MAG_CRB_REG_M             = 0x01,
      LSM303_REGISTER_MAG_MR_REG_M              = 0x02,
      LSM303_REGISTER_MAG_OUT_X_H_M             = 0x03,
      LSM303_REGISTER_MAG_OUT_X_L_M             = 0x04,
      LSM303_REGISTER_MAG_OUT_Z_H_M             = 0x05,
      LSM303_REGISTER_MAG_OUT_Z_L_M             = 0x06,
      LSM303_REGISTER_MAG_OUT_Y_H_M             = 0x07,
      LSM303_REGISTER_MAG_OUT_Y_L_M             = 0x08,
      LSM303_REGISTER_MAG_SR_REG_Mg             = 0x09,
      LSM303_REGISTER_MAG_IRA_REG_M             = 0x0A,
      LSM303_REGISTER_MAG_IRB_REG_M             = 0x0B,
      LSM303_REGISTER_MAG_IRC_REG_M             = 0x0C,
      LSM303_REGISTER_MAG_TEMP_OUT_H_M          = 0x31,
      LSM303_REGISTER_MAG_TEMP_OUT_L_M          = 0x32
    };
/*=========================================================================*/

/*=========================================================================
    MAGNETOMETER GAIN SETTINGS
    -----------------------------------------------------------------------*/
    enum class lsm303MagGain
    {
      LSM303_MAGGAIN_1_3                        = 0x20,  // +/- 1.3
      LSM303_MAGGAIN_1_9                        = 0x40,  // +/- 1.9
      LSM303_MAGGAIN_2_5                        = 0x60,  // +/- 2.5
      LSM303_MAGGAIN_4_0                        = 0x80,  // +/- 4.0
      LSM303_MAGGAIN_4_7                        = 0xA0,  // +/- 4.7
      LSM303_MAGGAIN_5_6                        = 0xC0,  // +/- 5.6
      LSM303_MAGGAIN_8_1                        = 0xE0   // +/- 8.1
    };	
/*=========================================================================*/

/*=========================================================================
    CHIP ID
    -----------------------------------------------------------------------*/
    #define LSM303_ID                     (0b11010100)
/*=========================================================================*/

class lsm303 {
	public:
		lsm303 () = default;
		lsm303(int bus);												/**< Create an interface object for an LSM303 on the given I2C bus. */

		bool setBus (int bus);											/**< Set the I2C bus to use. */
		bool begin(void);												/**< Initialize the LSM303. */
		bool readAll (void);											/**< Read all accelerometer and magnetometer values */
		void enableAutoRange(bool enable);								/**< Enable auto-ranging function (see data sheet). */
		void setMagGain(lsm303MagGain gain);							/**< Set the magnetometer gain. */
		map<char, double> getMagData (void);							/**< Get the scaled magnetometer data. There will be three fields, named x, y, and z. */
		map<char, double> getAccelData (void);							/**< Get the scaled accelerometer data. Fields named as for magnetometer. */
		map<char, int> getRawMagData (void);							/**< Get raw magnetometer data. Field names as for scaled data. */
		map<char, int> getRawAccelData (void);							/**< Get raw accelerometer data. Field names as for scaled data. */
		void setMagRegister(lsm303MagRegisters_t reg, uint8_t val);		/**< Set an arbitrary register on the chip. */
		uint8_t getMagRegister(lsm303MagRegisters_t reg);				/**< Read an arbitrary register on the chip. */
		void setAccelRegister(lsm303AccelRegisters_t reg, uint8_t val);	/**< Set an arbitrary register on the chip. */
		uint8_t getAccelRegister(lsm303AccelRegisters_t reg);			/**< Read an arbitrary register on the chip. */
		map<char, int> getMagOffset (void);								/**< Get the current offset for magnetometer data. Field names as for data. */
		map<char, int> getAccelOffset (void);							/**< Get the current offset for accelerometer data. Field names as for data. */
		map<char, double> getMagScale (void);							/**< Get the current scale factor for the magnetometer data. Field names as for data. */
		map<char, double> getAccelScale (void);							/**< Get the current scale factor for the accelerometer data. Field names as for data. */
		void setMagOffset (map<char, int>);								/**< Set magnetometer offsets. */
		void setAccelOffset (map<char, int>);							/**< Set accelerometer offsets. */
		void setMagScale (map<char, double>);							/**< Set magnetometer scale. */
		void setAccelScale (map<char, double>);							/**< Set accelerometer scale. */

	private:
		i2cClass			_bus;
		map<char, int> 		_accelData;   
		map<char, int>		_magData;
		map<char, double>  	_magScale;
		map<char, int>   	_magOffset;
		map<char, double>  	_accelScale;
		map<char, int>   	_accelOffset;

};


#endif