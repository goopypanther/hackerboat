/******************************************************************************
 * Hackerboat orientation input module
 * hal/orientationInput.hpp
 * This module reads orientation data. Its AHRS code is based on 
 * the Adafruit 9DOF library (https://github.com/adafruit/Adafruit_9DOF)
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

class HalTestHarness;

using namespace std;

enum class SensorOrientation : char {	/**< Choose the axis parallel to gravity when the system is level */
	SENSOR_AXIS_X_UP  = 'X',
	SENSOR_AXIS_Y_UP  = 'Y',
	SENSOR_AXIS_Z_UP  = 'Z',
	SENSOR_AXIS_X_DN  = 'x',
	SENSOR_AXIS_Y_DN  = 'y',
	SENSOR_AXIS_Z_DN  = 'z'
};

class OrientationInput : public InputThread {
	friend class HalTestHarness;
	public:	
		OrientationInput(SensorOrientation axis = SensorOrientation::SENSOR_AXIS_Z_UP);
		Orientation* getOrientation() {							/**< Get the last orientation recorded */
			return &_current;
		}
		bool init();											/**< initialize hardware */
		bool isValid() {return sensorsValid;};					/**< Check if the hardware connections are good */
		bool begin();											/**< Start the input thread */
		bool execute();											/**< Gather input	*/
		void setAxis(SensorOrientation axis) {_axis = axis;};	/**< Set the gravity axis */
		SensorOrientation getAxis () {return _axis;};			/**< Get the gravity axis */
		~OrientationInput () {
			this->kill(); 
			//if (myThread) delete myThread;
		}
		LSM303 compass { IMU_I2C_BUS };
	
	private:
		bool getData ();
		void mapAxes (map<char, double> data, double &x, double &y, double &z);
		void getAccelOrientation ();
		void getMagOrientation ();
		//L3GD20	gyro { IMU_I2C_BUS };
		
		std::thread *myThread;
		
		Orientation 				_current;
		bool 						sensorsValid = false;
		SensorOrientation			_axis = SensorOrientation::SENSOR_AXIS_Z_UP;
};

#endif
 
