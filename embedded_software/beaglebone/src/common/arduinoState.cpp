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
#include <cstring>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "arduinoState.hpp"
#include "boneState.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
//#include <BlackUART/BlackUART.h>

static logError *errLog = logError::instance();

// using namespace BlackLib;

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

hackerboatStateStorage &arduinoStateClass::storage() {
	static hackerboatStateStorage *arduinoStorage;

	if (!arduinoStorage) {
		arduinoStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(ARD_LOG_DB_FILE),
							    "ARDUINO",
							    { { "json", "TEXT"    } });
		arduinoStorage->createTable();
	}

	return *arduinoStorage;
}

bool arduinoStateClass::setCommand (arduinoModeEnum c) {
	if (!modeNames.valid(c))
		return false;
	command = c;
	return true;
}

bool arduinoStateClass::writeBoatMode(boatModeEnum m) {
	if (setBoatMode(m)) {
		json_t *ret = writeArduino("f_writeBoatMode", boneStateClass::modeNames.get(m));
		if (!ret)
			return false;
		json_decref(ret);
	} else return false;
	return true;
}

bool arduinoStateClass::writeCommand(void) {
	bool retVal = false;
	json_t *ret = writeArduino("f_writeCommand", modeNames.get(command));
	if (ret) {
		json_t *result = json_object_get(ret, "command");
		if (result) {
			std::string check = string(json_string_value(result));
			if (check == modeNames.get(command)) {
				retVal = true;
			} else {
				retVal = false;
			}
		} else {
			retVal = false;
		}
		json_decref(ret);
	} else retVal = false;
	return retVal;
}

bool arduinoStateClass::writeCommand(arduinoModeEnum c) {
	if (setCommand(c)) {
		return writeCommand();
	} else return false;
}

int16_t arduinoStateClass::writeThrottle(int16_t t) {
	throttle = t;
	return writeThrottle();
}

int16_t arduinoStateClass::writeThrottle(void) {
	json_t *ret = writeArduino("f_writeThrottle", std::to_string((int)throttle));
	if (ret) {
		json_t *result = json_object_get(ret, "throttle");
		if (result) {
			throttle = json_integer_value(result);
		}
		json_decref(ret);
	}
	return throttle;
}

double arduinoStateClass::writeHeadingTarget(void) {
	json_t *ret = writeArduino("f_writeHeadingTarget", std::to_string(headingTarget));
	if (ret) {
		json_t *result = json_object_get(ret, "headingTarget");
		if (result) {
			headingTarget = json_integer_value(result);
		}
		json_decref(ret);
	}
	return headingTarget;
}

double arduinoStateClass::writeHeadingDelta(double delta) {
	json_t *ret = writeArduino("f_writeHeadingDelta", std::to_string(delta));
	if (ret) {
		json_t *result = json_object_get(ret, "headingTarget");
		if (result) {
			headingTarget = json_integer_value(result);
		}
		json_decref(ret);
	}
	return headingTarget;
}

bool arduinoStateClass::heartbeat(void) {
	writeArduino("f_boatHeartBeat", "");
	return true;
}

bool arduinoStateClass::populate(void) {
	bool result;
	clock_gettime(CLOCK_REALTIME, &uTime);
	json_t *in = writeArduino("f_dumpCoreState", "");

	json_t *other = writeArduino("f_dumpOrientationState", "");
	json_object_update(in, other);
	json_decref(other);

	other = writeArduino("f_dumpInputState", "");
	json_object_update(in, other);
	json_decref(other);

	other = writeArduino("f_dumpRawInputState", "");
	json_object_update(in, other);
	json_decref(other);

	other = writeArduino("f_dumpOutputState", "");
	json_object_update(in, other);
	json_decref(other);
	
	if (in) {
		this->parse(in, false);
		result = this->isValid();
	} else {
		result = false;
		errLog->write("Arduino Serial", "Failed to parse incoming json from Arduino");
	}

	json_decref(in);
	return result;
}

bool arduinoStateClass::corePopulate(void) {
	bool result;
	clock_gettime(CLOCK_REALTIME, &uTime);
	json_t *in = writeArduino("f_dumpCoreState", "");

	if (in) {
		this->parse(in, false);
		result = this->isValid();
	} else {
		result = false;
		errLog->write("Arduino Serial", "Failed to parse incoming json from Arduino");
	}

	json_decref(in);
	return result;
}

bool arduinoStateClass::setMode(arduinoModeEnum m) {
	if (!modeNames.valid(m))
		return false;
	mode = m;
	return true;
}

bool arduinoStateClass::setBoatMode(boatModeEnum s) {
	if (!boneStateClass::modeNames.valid(s))
		return false;
	boat = s;
	return true;
}

json_t *arduinoStateClass::writeArduino(std::string func, std::string params) {
	std::string ret = "";
	std::string sent = "";
	json_t *result;
	json_error_t err;
	//FILE * ser;
	// BlackUART port(ARDUINO_REST_UART, ARDUINO_BAUD, ParityNo, StopOne, Char8);
	uint32_t cnt = 0;
	ssize_t bytesRead = 0;
	char buf[LOCAL_BUF_LEN];
	
	// attempt to open the serial port
	while (cnt < UART_TIMEOUT) {
		if (openArduinoSerial() != -1) break;	// if the serial port opened, leave. 
		usleep(1);
		cnt++;
	}
	
	// if we timed out, return an empty string
	if (cnt >= UART_TIMEOUT) {
		errLog->write("Arduino Serial", "Failed to open serial port for write");
		return NULL;
	// otherwise, make it a proper FILE structure
	} //else {
	//	ser = fdopen(ard_fd, "r+");
	//}
	
	// build the query string and write to the serial port
	std::ostringstream cmd;
	if (params.size() > 0) {
		cmd << func << "?params=" << params << "\r\n" << std::endl;
	} else {
		cmd << func << "\r\n" << std::endl;
	}
	sent = cmd.str();
	errLog->write("Arduino Command", func);
	errLog->write("Arduino Parameters", params);
	if (write(ard_fd, sent.c_str(), sent.length()) < (int)sent.length()) {
		errLog->write("Arduino Serial", "Write failed");
		return NULL;
	}
	//usleep(10000);
	
	// read the response from the serial port
	cnt = 0;
	while (ret.find("\r") == std::string::npos) {
		bytesRead = read(ard_fd, buf, LOCAL_BUF_LEN);
		if (bytesRead > 0) {
			ret += buf;
			cnt = 0;
			printf("(%d)%s;", bytesRead, buf);
			memset(buf, 0, LOCAL_BUF_LEN);
		} else {
			printf(".");
		}
		if (cnt >= UART_READ_TIMEOUT) {
			errLog->write("Arduino Serial", "Failed to read return value");
			printf("Exiting with timeout\n");
			closeArduinoSerial();
			return NULL;
		}
		usleep(1000);
		cnt++;
	}
	printf("\n");
	//tcflush(ard_fd, TCIOFLUSH);
	//tcdrain(ard_fd);
	
	closeArduinoSerial();
	errLog->write("Arduino Return Value", ret);
	result = json_loads(ret.c_str(), JSON_DECODE_ANY, &err);
	clock_gettime(CLOCK_REALTIME, &uTime);
	return result;
}
	
int arduinoStateClass::openArduinoSerial (void) {
	struct termios ard_attrib;
	
	ard_fd = open(ARDUINO_REST_TTY, O_RDWR | O_NONBLOCK | O_NOCTTY);
	//ard_fd = open(ARDUINO_REST_TTY, O_RDWR | O_NOCTTY);
	if (ard_fd == -1) return ard_fd;	
	if (tcgetattr(ard_fd, &ard_attrib) < 0) {
		errLog->write("Arduino Serial", "Unable to get serial properties");
		closeArduinoSerial();
		return ard_fd;
	}
	cfsetspeed(&ard_attrib, B115200);
	//cfsetspeed(&ard_attrib, B230400);
	cfmakeraw(&ard_attrib);
	ard_attrib.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	ard_attrib.c_cflag &= ~(PARENB | CSTOPB | CSIZE);		// no parity, one stop bit
	ard_attrib.c_cflag |= (CLOCAL | CREAD | CS8);			// ignore modem status lines, enable receiver, 8 bits per byte
	ard_attrib.c_iflag &= ~(IXON | IXOFF | IXANY);			// turn off all flow control
	ard_attrib.c_iflag &= ~(ICRNL);							// turn off line ending translation					
	ard_attrib.c_oflag &= ~(OPOST);							// turn off post processing of output
	ard_attrib.c_cc[VMIN] = 0;								// this sets the timeouts for the read() operation to minimum
	ard_attrib.c_cc[VTIME] = 0;
	if (tcsetattr(ard_fd, TCSANOW, &ard_attrib) < 0) {
		closeArduinoSerial();
		errLog->write("Arduino Serial", "Unable to set serial properties");
		return ard_fd;
	}
	
	return ard_fd;
}

void arduinoStateClass::closeArduinoSerial (void) {
	tcflush(ard_fd, TCIOFLUSH);
	close(ard_fd);
	ard_fd = -1;
}
