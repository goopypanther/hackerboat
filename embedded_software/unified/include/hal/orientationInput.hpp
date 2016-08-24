/******************************************************************************
 * Hackerboat orientation input module
 * hal/orientationInput.hpp
 * This module reads orientation data. Its AHRS code is based on Adafruit's
 * simple AHRS code. 
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ORIENTATIONINPUT_H
#define ORIENTATIONINPUT_H

#include <string>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "orientation.hpp"
#include "hal/config.h"
#include "hal/drivers/lsm303.hpp"
#include "hal/drivers/l3gd20.hpp"
#include "hal/inputThread.hpp"

class OrientationInput : public InputThread {
	public:
		OrientationInput(void);			
		Orientation* getOrientation() {return &_current;};	/**< Get the last orientation recorded */
		
	private:
		LSM303	gyro { IMU_I2C_BUS };
		L3GD20	compass { IMU_I2C_BUS };
		Orientation _current;
};

#endif
 