/******************************************************************************
 * Hackerboat sim module
 * hal_sim/HackerboatHALsim.cpp
 * This module pretends to be the HAL in order to provide a simulated environment
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Apr 2017
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include "HackerboatHALsim.hpp"
#include "hal/orientationInput.hpp"
#include "hal/servo.hpp"
#include "hal/throttle.hpp"
#include "hal/gpsdInput.hpp"
#include "orientation.hpp"
#include "location.hpp"
#include "rapidjson/rapidjson.h"

using namespace std;
using namespace rapidjson;

HackerboatHALsim::HackerboatHALsim () {
	bool argsStrFnd = false;
	for (auto a : Args::getargs()->args()) {
		if (argsStrFnd) {
			load(a);
			break;
		}
		if (a == "--sim-conf") argsStrFnd = true;
	}
	if (!argStrFnd) load();
	this->myThread = new std::thread (HackerboatHALsim::SimThreadRunner(this));
	myThread->detach();
	return;
}

bool HackerboatHALsim::load () {
	return load(SIM_CONF_DEFAULT);
}
												
bool HackerboatHALsim::load (const string &path) {

	return true;
}
								
bool HackerboatHALsim::execute() {
}

bool HackerboatHALsim::setRudder (double rudder);
bool HackerboatHALsim::setThrottle (double throttle);

OrientationInput::OrientationInput(SensorOrientation axis) : _axis(axis) {period = IMU_READ_PERIOD;}
bool OrientationInput::init() {return true;}
bool OrientationInput::execute() {_current = HackerboatHALsim::getsim()->currentOrientation;}

bool OrientationInput::begin() {
	this->myThread = new std::thread (InputThread::InputThreadRunner(this));
	myThread->detach();
	LOG(INFO) << "Successfully initialized orientation subsystem";
	return true;
}


GPSdInput::GPSdInput() {};					
GPSdInput::GPSdInput(string host, int port) {};
bool GPSdInput::setHost (string host) {return true;}
bool GPSdInput::setPort (int port) {return true;}
bool GPSdInput::connect () {return true;}
bool GPSdInput::disconnect () {return true;}
bool GPSdInput::isConnected () {return true;}
int GPSdInput::pruneAIS(Location loc) {return 0;}

bool GPSdInput::begin(){
	this->myThread = new std::thread (InputThread::InputThreadRunner(this));
	myThread->detach();
	LOG(INFO) << "Successfully initialized GPS/AIS subsystem";
	return true;
}

bool GPSdInput::execute() {
	_lastFix.copy(HackerboatHALsim::getsim()->currentFix);
	_gpsAvgList.emplace_front(_lastFix);
	if (_gpsAvgList.size() < GPS_AVG_LEN) {
		_gpsAvgList.pop_back();
	}
	return true;
}

std::map<int, AISShip>* GPSdInput::getData() {
	return NULL;
}

std::map<int, AISShip> GPSdInput::getData(AISShipType type) {
	std::map<int, AISShip> result;
	return result;
}

AISShip* GPSdInput::getData(int MMSI) {
	return NULL;
}
			
AISShip* GPSdInput::getData(string name) {
	return NULL;
}

GPSFix GPSdInput::getAverageFix() {
	double lattot = 0, lontot = 0;
	GPSFix result;
	for (auto &thisfix: _gpsAvgList) {
		lattot += thisfix.fix.lat;
		lontot += thisfix.fix.lon;
	}
	result.fix.lat = lattot/_gpsAvgList.size();
	result.fix.lon = lontot/_gpsAvgList.size();
	return result;
}

Servo::Servo() {_attached = true;}
bool Servo::attach(int port, int pin, long min, long max, long freq) {return true;}		
void Servo::detach() {}
bool Servo::setFrequency(unsigned long freq) {return true;}
bool Servo::setMax (unsigned long max) {return true;}
bool Servo::setMin (unsigned long min) {return true;}
		
bool Servo::write(double value) {
	if (value > 100.0) return false;
	if (value < -100.0) return false;
	long span = _max - _min;	// this should probably be pre-calculated
	// We divide span by 200.0 to get the number of nanosecconds for each count of the input span
	// We add 100 to the input value so it runs from {0 - 200} rather than {-100 - 100}
	_val = floor((value+100)*(span/200.0)) + _min;
	return HackerboatHALsim::getsim()->setRudder(value);
	return true;
}

bool Servo::writeMicroseconds(unsigned long value) {
	if (value < _min) return false;
	if (value > _max) return false;
	_val = value;
	long span = _max - _min;
	double value = ((double)(_val - _min)/(double)(span/200.0)) - 100.0;
	return this->write(value);
}

double Servo::read() {
	int span = _max - _min;
	int posn = _val - _min;
	return ((double)posn*(200.0/(double)span)) - 100.0;
}

unsigned long Servo::readMicroseconds() {
	return _val/1000;
}

bool Throttle::setThrottle(int throttle) {
	return HackerboatHALsim::getsim()->setThrottle(throttle);
}

double Throttle::getMotorCurrent() {return 0;}
double Throttle::getMotorVoltage() {return 0;}
