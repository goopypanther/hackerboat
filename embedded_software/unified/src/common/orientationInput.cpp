/******************************************************************************
 * Hackerboat orientation input module
 * hal/orientationInput.cpp
 * This module reads orientation data. Its AHRS code is based on 
 * the Adafruit 9DOF library (https://github.com/adafruit/Adafruit_9DOF)
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <cassert>
#include <cmath>
#include "orientation.hpp"
#include "hal/config.h"
#include "hal/drivers/lsm303.hpp"
#include "hal/drivers/l3gd20.hpp"
#include "hal/inputThread.hpp"
#include "hal/orientationInput.hpp"
#include "twovector.hpp"


OrientationInput::OrientationInput(SensorOrientation axis) : _axis(axis) {
	sensorsValid = init();
}		

bool OrientationInput::init() {
	sensorsValid = (gyro.begin() & compass.begin());
	return sensorsValid;
}	
				
bool OrientationInput::begin() {
	if (this->init()) {
		this->myThread = new std::thread (InputThread::InputThreadRunner(this));
		myThread->detach();
		return true;
	}
	return false;
}

bool OrientationInput::execute() {
	if (!lock && (!lock.try_lock_for(IMU_LOCK_TIMEOUT))) return false;
	if (!getData()) return false;
	getAccelOrientation();
	getMagOrientation();
	lock.unlock();
	return true;
}		

bool OrientationInput::getData () {
	this->setLastInputTime();
	return (compass.readAll() && gyro.read());
}


void OrientationInput::mapAxes (map<char, double> data, double &x, double &y, double &z) {
	// assign the axis data, making sure to keep it all right hand ruled
	switch (_axis) {
		case (SensorOrientation::SENSOR_AXIS_X_UP):
			x = data['y'];
			y = data['z'];
			z = data['x'];
			break;
		case (SensorOrientation::SENSOR_AXIS_Y_UP):
			x = data['z'];
			y = data['x'];
			z = data['y'];
			break;
		case (SensorOrientation::SENSOR_AXIS_Z_UP):
			x = data['x'];
			y = data['y'];
			z = data['z'];
			break;
		case (SensorOrientation::SENSOR_AXIS_X_DN):
			x = data['y'];
			y = data['z'];
			z = -data['x'];
			break;
		case (SensorOrientation::SENSOR_AXIS_Y_DN):
			x = data['x'];
			y = data['z'];
			z = -data['y'];
			break;
		case (SensorOrientation::SENSOR_AXIS_Z_DN):
			x = data['y'];
			y = data['x'];
			z = -data['z'];
			break;
		default:
			break;
	}
}

/*!
 * @brief populate the pitch and roll fields in the _current field 
 *
 * Converts a set of accelerometer readings into pitch and roll
 * angles. The returned values indicate the orientation of the
 * accelerometer with respect to the "down" vector which it is
 * sensing. Must be called before getMagOrientation(), because 
 * that function depends on the roll and pitch values here.
 */
 
void OrientationInput::getAccelOrientation () {
	double x, y, z, zSign;
	
	mapAxes(compass.getAccelData(), x, y, z);
	zSign = (z >= 0) ? 1.0 : -1.0;
	
	/* roll: Rotation around the longitudinal axis (the plane body, 'X axis'). -90<=roll<=90    */
	/* roll is positive and increasing when moving downward                                     */
	/*                                                                                          */
	/*                                 y                                                        */
	/*             roll = atan(-----------------)                                               */
	/*                          sqrt(x^2 + z^2)                                                 */
	/* where:  x, y, z are returned value from accelerometer sensor                             */

	_current.roll = TwoVector::rad2deg(atan2(y, sqrt((x*x)+(z*z))));
	
	/* pitch: Rotation around the lateral axis (the wing span, 'Y axis'). -180<=pitch<=180)     */
	/* pitch is positive and increasing when moving upwards                                     */
	/*                                                                                          */
	/*                                 x                                                        */
	/*             roll = atan(-----------------)                                               */
	/*                          sqrt(y^2 + z^2)                                                 */
	/* where:  x, y, z are returned value from accelerometer sensor                             */
	
	_current.pitch = TwoVector::rad2deg(atan2(x, zSign*sqrt(y*y+z*z)));
	_current.normalize();
	
}

void OrientationInput::getMagOrientation () {
	double x, xRaw, y, yRaw, zRaw;
	
	mapAxes(compass.getMagData(), xRaw, yRaw, zRaw);
	
	// mag tilt compensation
	double cosRoll = cos(TwoVector::deg2rad(_current.roll));
	double sinRoll = sin(TwoVector::deg2rad(_current.roll));
	double cosPitch = cos(TwoVector::deg2rad(_current.pitch));
	double sinPitch = sin(TwoVector::deg2rad(_current.pitch));
	x = xRaw*cosPitch + zRaw*cosPitch;
	y = xRaw*sinRoll*sinPitch + yRaw*cosRoll - zRaw*sinRoll*cosPitch;
	
	// magnetic heading
	_current.heading = TwoVector::rad2deg(atan2(y,x));
	_current.normalize();
}

