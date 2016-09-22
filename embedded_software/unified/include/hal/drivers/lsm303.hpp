/******************************************************************************
 * Hackerboat Beaglebone LSM303 module
 * hal/drivers/LSM303.hpp
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
#include <map>
#include <vector>
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
    enum class LSM303AccelRegistersEnum : char
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
    
    enum class LSM303MagRegistersEnum : char
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
    ACCELEROMETER SPEED SETTINGS
    -----------------------------------------------------------------------*/
    enum class LSM303AccelSpeedEnum : char
    {
      LSM303_ACCEL_PWRDN						= 0x07,  // Accelerometer off
      LSM303_ACCEL_1_HZ							= 0x17,  // 1 Hz
      LSM303_ACCEL_10_HZ						= 0x27,  // 10 Hz
      LSM303_ACCEL_25_HZ						= 0x37,  // 25 Hz
      LSM303_ACCEL_50_HZ						= 0x47,  // 50 Hz
      LSM303_ACCEL_100_HZ						= 0x57,  // 100 Hz
      LSM303_ACCEL_200_HZ						= 0x67,  // 200 Hz
      LSM303_ACCEL_400_HZ						= 0x77   // 400 Hz
    };	
/*=========================================================================*/

/*=========================================================================
    MAGNETOMETER GAIN SETTINGS
    -----------------------------------------------------------------------*/
    enum class LSM303MagGainEnum : char
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

using namespace std;

class LSM303 {
	public:
		LSM303 () = default;
		LSM303(uint8_t bus) {setBus(bus);};								/**< Create an interface object for an LSM303 on the given I2C bus. */
		bool setBus (uint8_t bus);										/**< Set the I2C bus to use. */
		bool begin();													/**< Initialize the LSM303. */
		bool readAll () {return (readAccel() && readMag());};			/**< Read all accelerometer and magnetometer values */
		bool readMag ();
		bool readAccel ();
		bool readTemp ();
		void setMagGain(LSM303MagGainEnum gain);						/**< Set the magnetometer gain. */
	
		bool setMagRegister(LSM303MagRegistersEnum reg, uint8_t val);	/**< Set an arbitrary register on the chip. */
		uint8_t getMagRegister(LSM303MagRegistersEnum reg);				/**< Read an arbitrary register on the chip. */
		bool setAccelRegister(LSM303AccelRegistersEnum reg, uint8_t val);	/**< Set an arbitrary register on the chip. */
		uint8_t getAccelRegister(LSM303AccelRegistersEnum reg);			/**< Read an arbitrary register on the chip. */
		
		map<char, double> getMagData ();								/**< Get the scaled magnetometer data. There will be three fields, named x, y, and z. */
		map<char, double> getAccelData ();								/**< Get the scaled accelerometer data. Fields named as for magnetometer. */
		double getTempData ();											/**< Get the scaled temperature data */
		map<char, int> getRawMagData () {return _magData;};				/**< Get raw magnetometer data. Field names as for scaled data. */
		map<char, int> getRawAccelData () {return _accelData;};			/**< Get raw accelerometer data. Field names as for scaled data. */
		int getRawTempData () {return _tempData;};						/**< Get raw temperature data. */
		map<char, int> getMagOffset () {return _magOffset;};			/**< Get the current offset for magnetometer data. Field names as for data. */
		map<char, int> getAccelOffset () {return _accelOffset;};		/**< Get the current offset for accelerometer data. Field names as for data. */
		int getTempOffset () {return _tempOffset;};						/**< Get the offset used for the temperature data */
		map<char, double> getMagScale () {return _magScale;};			/**< Get the current scale factor for the magnetometer data. Field names as for data. */
		map<char, double> getAccelScale () {return _accelScale;};		/**< Get the current scale factor for the accelerometer data. Field names as for data. */
		double getTempScale () {return _tempScale;};					/**< Get the scaling factor used for the temperature */
		bool setMagOffset (map<char, int> offset);						/**< Set magnetometer offsets. */
		bool setAccelOffset (map<char, int> offset);					/**< Set accelerometer offsets. */
		bool setMagScale (map<char, double> scale);						/**< Set magnetometer scale. */
		bool setAccelScale (map<char, double> scale);					/**< Set accelerometer scale. */
		void setTempOffset (int offset) {_tempOffset = offset;};		/**< Set temperature offset */
		void setTempScale (double scale) {_tempScale = scale;};			/**< Set temperature scale */
		
	private:	
		bool 				setReg (uint8_t addr, uint8_t reg, uint8_t val);
		int16_t				getReg (uint8_t addr, uint8_t reg);
		int					_bus;
		int					_tempData = 0;
		map<char, int> 		_accelData = {{'x',0},{'y',0},{'z',0}};   
		map<char, int>		_magData = {{'x',0},{'y',0},{'z',0}};
		int 				_tempOffset = 0;
		double				_tempScale = 1;
		map<char, double>  	_magScale = {{'x',1},{'y',1},{'z',1}};
		map<char, int>   	_magOffset = {{'x',0},{'y',0},{'z',0}};
		map<char, double>  	_accelScale = {{'x',1},{'y',1},{'z',1}};
		map<char, int>   	_accelOffset = {{'x',0},{'y',0},{'z',0}};

};


#endif