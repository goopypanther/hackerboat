/******************************************************************************
 * Hackerboat orientation input module
 * hal/orientationInput.hpp
 * This module reads orientation data
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

class orientationInputClass : public inputThreadClass {
	public:
		orientationInputClass(void);			
		orientation getOrientation(void);		/**< Get the last orientation recorded */
		
	private:
		lsm303	gyro(IMU_I2C_BUS);
		l3gd20	compass(IMU_I2C_BUS);
		orientation _current;
};

#endif
 