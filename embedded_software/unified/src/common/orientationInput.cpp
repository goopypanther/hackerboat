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
#include "easylogging++.h"

using namespace std;

OrientationInput::OrientationInput(SensorOrientation axis) : _axis(axis) {
	period = Conf::get()->imuReadPeriod();
}		

bool OrientationInput::init() {
	LOG(INFO) << "Creating new OrientationInput object";
	sensorsValid = (/*gyro.begin() & */compass.begin());
	compass.setMagOffset ( Conf::get()->imuMagOffset() );
	compass.setMagScale ( Conf::get()->imuMagScale() );
	LOG_IF(!sensorsValid, ERROR) << "Failed to initialize orientation subsystem";
	return sensorsValid;
}	
				
bool OrientationInput::begin() {
	if (this->init()) {
		this->myThread = new std::thread (InputThread::InputThreadRunner(this));
		myThread->detach();
		LOG(INFO) << "Successfully initialized orientation subsystem";
		return true;
	}
	return false;
}

bool OrientationInput::execute() {
	//if (!lock && (!lock.try_lock_for(IMU_LOCK_TIMEOUT))) {
	//	LOG(ERROR) << "Failed to lock data for OrientationInput";
	//	return false;
	//}
	if (!getData()) {
		//lock.unlock();
		return false;
	}
	getAccelOrientation();
	getMagOrientation();
	//lock.unlock();
	return true;
}		

bool OrientationInput::getData () {
	this->setLastInputTime();
	return (compass.readAll()/* && gyro.read()*/);
}


void OrientationInput::mapAxes (tuple<double, double, double> data, double &x, double &y, double &z) {
	// assign the axis data, making sure to keep it all right hand ruled
	// only the Z_UP direction has been tested. 
	switch (_axis) {
		case (SensorOrientation::SENSOR_AXIS_X_UP):
			x = std::get<1>(data);
			y = std::get<2>(data);
			z = std::get<0>(data);
			break;
		case (SensorOrientation::SENSOR_AXIS_Y_UP):
			x = std::get<2>(data);
			y = std::get<0>(data);
			z = std::get<1>(data);
			break;
		case (SensorOrientation::SENSOR_AXIS_Z_UP):
			x = std::get<0>(data);
			y = std::get<1>(data);
			z = std::get<2>(data);
			break;
		case (SensorOrientation::SENSOR_AXIS_X_DN):
			x = std::get<1>(data);
			y = std::get<2>(data);
			z = -std::get<0>(data);
			break;
		case (SensorOrientation::SENSOR_AXIS_Y_DN):
			x = std::get<0>(data);
			y = std::get<2>(data);
			z = -std::get<1>(data);
			break;
		case (SensorOrientation::SENSOR_AXIS_Z_DN):
			x = std::get<1>(data);
			y = std::get<0>(data);
			z = std::get<2>(data);
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
 * sensing. 
 */
 
void OrientationInput::getAccelOrientation () {
	double x, y, z;
	
	mapAxes(compass.getAccelData(), x, y, z);
	
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
	
	_current.pitch = TwoVector::rad2deg(atan2(x, z));
	_current.normalize();
	LOG_EVERY_N(100, DEBUG) << "Orientation after getAccelOrientation: " << _current;
	LOG_EVERY_N(1000, INFO) << "Orientation after getAccelOrientation: " << _current;
}

void OrientationInput::getMagOrientation () {
	double xRaw, yRaw, zRaw;
	double Axraw, Ayraw, Azraw;
	
	mapAxes(compass.getMagData(), xRaw, yRaw, zRaw);
	mapAxes(compass.getAccelData(), Axraw, Ayraw, Azraw);
	
	// mag tilt compensation, per http://www.cypress.com/file/130456/download
	double Atotal = sqrt(Axraw*Axraw + Ayraw*Ayraw + Azraw*Azraw);
	double Ax = Axraw/Atotal;
	double Ay = Ayraw/Atotal;
	double B = 1 - (Ax*Ax);
	double C = Ax*Ay;
	double D = sqrt(1 - (Ax*Ax) - (Ay*Ay));
	double x = xRaw*B - yRaw*C - zRaw*Ax*D;		// Equation 18
	double y = yRaw*D - zRaw*Ay;				// Equation 19
		
	// magnetic heading
	_current.heading = TwoVector::rad2deg(atan2(y,x));
	_current.normalize();
	LOG_EVERY_N(100, DEBUG) << "Orientation after getMagOrientation: " << _current;
	LOG_EVERY_N(1000, INFO) << "Orientation after getMagOrientation: " << _current;
}

