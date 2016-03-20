/******************************************************************************
 * Hackerboat Arduino State module
 * arduinoState.cpp
 * This modules is compiled into the other modules to give a common interface
 * to the Arduino. 
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <string>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "arduinoState.hpp"
#include "boneState.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"
#include <BlackUART/BlackUART.h>

static logError *errLog = logError::instance();

using namespace BlackLib;

const enumerationNameTable<arduinoModeEnum> arduinoStateClass::modeNames = {
	"PowerUp", 
	"Armed", 
	"SelfTest", 
	"Disarmed", 
	"Active", 
	"LowBattery", 
	"Fault", 
	"SelfRecovery", 
	"ArmedTest", 
	"ActiveRudder", 
	"None"
};

json_t *arduinoStateClass::pack (bool seq) const {
	json_t *output = json_object();
	if (seq) json_object_set_new(output, "sequenceNum", json_integer(_sequenceNum));
	if (json_object_add(output, "ooooioo ffff oo IIII o fiiifi fff fff fff fff o oooooo o IIo",
			    "popStatus", json(popStatus),
			    "uTime", json(uTime),
			    "mode", json(mode),
			    "command", json(command),
			    "throttle", (int)throttle,
			    "boat", json(boat),
			    "orientation", orientation.pack(),

			    "headingTarget", double(headingTarget),
			    "internalVoltage", double(internalVoltage),
			    "batteryVoltage", double(batteryVoltage),
			    "motorVoltage", double(motorVoltage),

			    "enbButton", json_boolean(enbButton),
			    "stopButton", json_boolean(stopButton),

			    "timeSinceLastPacket", json_int_t(timeSinceLastPacket),
			    "timeOfLastPacket", json_int_t(timeOfLastPacket),
			    "timeOfLastBoneHB", json_int_t(timeOfLastBoneHB),
			    "timeOfLastShoreHB", json_int_t(timeOfLastShoreHB),

			    "faultString", json(faultString),

			    "rudder", double(rudder),
			    "rudderRaw", int(rudderRaw),
			    "internalVoltageRaw", int(internalVoltageRaw),
			    "motorVoltageRaw", int(motorVoltageRaw),
			    "motorCurrent", double(motorCurrent),
			    "motorCurrentRaw", int(motorCurrentRaw),

			    "Kp", double(Kp),
			    "Ki", double(Ki),
			    "Kd", double(Kd),

			    "magX", double(magX),
			    "magY", double(magY),
			    "magZ", double(magZ),

			    "accX", double(accX),
			    "accY", double(accY),
			    "accZ", double(accZ),

			    "gyroX", double(gyroX),
			    "gyroY", double(gyroY),
			    "gyroZ", double(gyroZ),

			    "horn", json(horn),

			    "motorDirRly", json(motorDirRly),
			    "motorWhtRly", json(motorWhtRly),
			    "motorYlwRly", json(motorYlwRly),
			    "motorRedRly", json(motorRedRly),
			    "motorRedWhtRly", json(motorRedWhtRly),
			    "motorRedYlwRly", json(motorRedYlwRly),

			    "servoPower", json(servoPower),

			    "startStopTime", json_int_t(startStopTime),
			    "startModeTime", json_int_t(startModeTime),
			    "originMode", json(originMode)) != 0) {
		json_decref(output);
		return NULL;
	}

	return output;
}

bool arduinoStateClass::parse (json_t *input, bool seq = true) {
	json_t *tmp;
	if (seq) {
		tmp = json_object_get(input, "sequenceNum");
		if (!json_is_integer(tmp))
			return false;
		_sequenceNum = json_integer_value(tmp);
	}

	Mode tmp_command;
	boneStateClass::Mode tmp_boat;
	double tmp_float;

	if (!::parse(json_object_get(input, "popStatus"), &popStatus) ||
	    !::parse(json_object_get(input, "mode"), &mode) ||
	    !::parse(json_object_get(input, "command"), &tmp_command) ||
	    !::parse(json_object_get(input, "boat"), &tmp_boat))
		return false;
	setCommand(tmp_command);
	setBoatMode(tmp_boat);

#define GET_VAR(var) if(!::parse(json_object_get(input, #var), &var)) { return false; }

	GET_VAR(uTime);

	GET_VAR(throttle);

	tmp = json_object_get(input, "orientation");
	if (!tmp || !orientation.parse(tmp))
		return false;

	if (!::parse(json_object_get(input, "headingTarget"), &tmp_float)) return false;
	headingTarget = fmod(tmp_float, 360.0);

	GET_VAR(internalVoltage);
	GET_VAR(batteryVoltage);
	GET_VAR(motorVoltage);
	GET_VAR(enbButton);
	GET_VAR(stopButton);
	GET_VAR(timeSinceLastPacket);
	GET_VAR(timeOfLastPacket);
	GET_VAR(timeOfLastBoneHB);
	GET_VAR(timeOfLastShoreHB);
	GET_VAR(faultString);
	GET_VAR(rudder);
	GET_VAR(rudderRaw);
	GET_VAR(internalVoltageRaw);
	GET_VAR(motorVoltageRaw);
	GET_VAR(motorCurrent);
	GET_VAR(motorCurrentRaw);

	// rudder PID constants
	GET_VAR(Kp);
	GET_VAR(Ki);
	GET_VAR(Kd);
	
	// magnetometer data
	GET_VAR(magX);
	GET_VAR(magY);
	GET_VAR(magZ);
	
	// acceleration data
	GET_VAR(accX);
	GET_VAR(accY);
	GET_VAR(accZ);
	
	// gyro data
	GET_VAR(gyroX);
	GET_VAR(gyroY);
	GET_VAR(gyroZ);
	
	// relay states
	GET_VAR(horn);
	GET_VAR(motorDirRly);
	GET_VAR(motorWhtRly);
	GET_VAR(motorYlwRly);
	GET_VAR(motorRedRly);
	GET_VAR(motorRedWhtRly);
	GET_VAR(motorRedYlwRly);
	GET_VAR(servoPower);
	
	// state timers
	GET_VAR(startStopTime);
	GET_VAR(startModeTime);

	if (!::parse(json_object_get(input, "originMode"), &originMode))
		return false;

	return this->isValid();
}
	
bool arduinoStateClass::isValid (void) const {
	if (!orientation.isValid()) return false;
	if (!modeNames.valid(mode)) return false;
	if (!modeNames.valid(command)) return false;
	if (!boneStateClass::modeNames.valid(boat)) return false;
	return true;
}

bool arduinoStateClass::setCommand (arduinoModeEnum c) {
	if ((c > arduinoStateCount) || (c < 0)) return false;
	command = c;
	return true;
}

bool arduinoStateClass::writeBoatMode(boatModeEnum s) {
	if (setBoatMode(s)) {
		std::string ret = write("writeBoatMode", std::string.to_string((int)bone));
	} else return false;
	return true;
}

bool arduinoStateClass::writeCommand(void) {
	bool retVal = false;
	json_t *ret = write("writeCommand", enumerationNameTable::get(command));
	if (ret) {
		json_t *result = json_object_get(ret, "command");
		if (result) {
			std::string check = string(json_string_value(result));
			if (check == enumerationNameTable::get(command)) {
				retVal = true;
			} else {
				retVal = false;
			}
			free(result);
		} else {
			retVal = false;
		}
	} else retVal = false;
	free(ret);
	return retVal;
}

bool arduinoStateClass::writeCommand(arduinoModeEnum c) {
	if (setCommand(c)) {
		return writeCommand();
	} else return false;
}

int16_t writeThrottle(int16_t t) {
	throttle = t;
	return writeThrottle();
}

int16_t arduinoStateClass::writeThrottle(void) {
	bool retVal = false;
	json_t *ret = write("writeThrottle", std::string.to_string((int)throttle));
	if (ret) {
		json_t *result = json_object_get(ret, "throttle");
		if (result) {
			throttle = json_integer_value(result);
			free(result);
		}
	}
	free(ret);
	return throttle;
}

double arduinoStateClass::writeHeadingTarget(void) {
	json_t *ret = write("writeHeadingTarget", std::string.to_string(headingTarget));
	if (ret) {
		json_t *result = json_object_get(ret, "headingTarget");
		if (result) {
			headingTarget = json_integer_value(result);
			free(result);
		}
	}
	free(ret);
	return headingTarget;
}

double arduinoStateClass::writeHeadingDelta(double delta) {
	json_t *ret = write("writeHeadingDelta", std::string.to_string(delta));
	if (ret) {
		json_t *result = json_object_get(ret, "headingTarget");
		if (result) {
			headingTarget = json_integer_value(result);
			free(result);
		}
	}
	free(ret);
	return headingTarget;
}

bool arduinoStateClass::heartbeat(void) {
	write("boneHeartBeat", "");
	return true;
}

bool arduinoStateClass::populate(void) {
	bool result = true;
	json_error_t *err;
	bool result;
	json_t *in = write("dumpCoreState", "");
	json_t *other = write("dumpOrientationState", "");
	json_update(in, other);
	other = write("dumpInputState", "");
	json_update(in, other);
	other = write("dumpRawInputState", "");
	json_update(in, other);
	other = write("dumpOutputState", "");
	json_update(in, other);
	
	if (in) {
		this->parse(in, false);
		result = this->isValid();
	} else {
		result = false;
		errLog->write("Arduino Serial", "Failed to parse incoming json from Arduino");
	}
	free(in);
	free(other);
	free(err);
	return result;
}

bool arduinoStateClass::setMode(arduinoModeEnum s) {
	if ((s > arduinoStateCount) || (s < 0)) return false; 
	mode = s;
	return true;
}

bool arduinoStateClass::setBoatMode(boatModeEnum s) {
	if ((s > boneStateClass::boneStateCount) || (s < 0)) return false;
	boat = s;
	return true;
}

json_t *arduinoStateClass::write(string func, string params) {
	std::string ret;
	json_t *result;
	json_err_t *err;
	BlackUART port(ARDUINO_REST_UART, ARDUINO_BAUD, ParityNo, StopOne, Char8);
	uint32_t cnt = 0;
	
	// attempt to open the serial port
	while (cnt < UART_TIMEOUT) {
		if (port.open(ReadWrite)) break;
		usleep(1);
		cnt++;
	}
	
	// if we timed out, return an empty string
	if (cnt >= UART_TIMEOUT) {
		port.close();
		errLog->write("Arduino Serial", "Failed to open serial port for write");
		return NULL;
	}
	
	port.setReadBufferSize(LOCAL_BUF_LEN);
	port << func << "?params=" << params << "\r\n" << std::endl;
	port >> ret;
	port.close();
	if (ret == BlackLib::UART_READ_FAILED) {
		errLog->write("Arduino Serial", "Failed to read return value");
		return NULL;
	}
	result = json_loads(ret.c_str(), JSON_DECODE_ANY, err);
	return result;
}
	
