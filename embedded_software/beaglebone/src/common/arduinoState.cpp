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
#include <sys/ioctl.h>
#include <asm/termbits.h>

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

static logError *errLog = logError::instance();

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
	int packResult = 0;
	if (seq) json_object_set_new(output, "sequenceNum", json_integer(_sequenceNum));
	packResult += json_object_set_new(output, "popStatus", json(popStatus));
	packResult += json_object_set_new(output, "uTime", json(uTime));
	packResult += json_object_set_new(output, "mode", json(mode));
	packResult += json_object_set_new(output, "command", json(command));
	packResult += json_object_set_new(output, "throttle", json_integer((int)throttle));
	packResult += json_object_set_new(output, "boat", json(boat));
	packResult += json_object_set_new(output, "orientation", orientation.pack());
	packResult += json_object_set_new(output, "headingTarget", json_real(headingTarget));
	packResult += json_object_set_new(output, "internalVoltage", json_real(internalVoltage));
	packResult += json_object_set_new(output, "batteryVoltage", json_real(batteryVoltage));
	packResult += json_object_set_new(output, "motorVoltage", json_real(motorVoltage));
	packResult += json_object_set_new(output, "enbButton", json_boolean(enbButton));
	packResult += json_object_set_new(output, "stopButton", json_boolean(stopButton));
	packResult += json_object_set_new(output, "timeSinceLastPacket", json_integer(timeSinceLastPacket));
	packResult += json_object_set_new(output, "timeOfLastPacket", json_integer(timeOfLastPacket));
	packResult += json_object_set_new(output, "timeOfLastBoneHB", json_integer(timeOfLastBoneHB));
	packResult += json_object_set_new(output, "timeOfLastShoreHB", json_integer(timeOfLastShoreHB));
	packResult += json_object_set_new(output, "faultString", json(faultString));
	packResult += json_object_set_new(output, "rudder", json_real(rudder));
	packResult += json_object_set_new(output, "rudderRaw", json_integer(rudderRaw));
	packResult += json_object_set_new(output, "internalVoltageRaw", json_integer(internalVoltageRaw));
	packResult += json_object_set_new(output, "motorVoltageRaw", json_integer(motorVoltageRaw));
	packResult += json_object_set_new(output, "motorCurrent", json_real(motorCurrent));
	packResult += json_object_set_new(output, "motorCurrentRaw", json_integer(motorCurrentRaw));
	packResult += json_object_set_new(output, "Kp", json_real(Kp));
	packResult += json_object_set_new(output, "Ki", json_real(Ki));
	packResult += json_object_set_new(output, "Kd", json_real(Kd));
	packResult += json_object_set_new(output, "magX", json_real(magX));
	packResult += json_object_set_new(output, "magY", json_real(magY));
	packResult += json_object_set_new(output, "magZ", json_real(magZ));
	packResult += json_object_set_new(output, "accX", json_real(accX));
	packResult += json_object_set_new(output, "accY", json_real(accY));
	packResult += json_object_set_new(output, "accZ", json_real(accZ));
	packResult += json_object_set_new(output, "gyroX", json_real(gyroX));
	packResult += json_object_set_new(output, "gyroY", json_real(gyroY));
	packResult += json_object_set_new(output, "gyroZ", json_real(gyroZ));
	packResult += json_object_set_new(output, "horn", json(horn));
	packResult += json_object_set_new(output, "motorDirRly", json(motorDirRly));
	packResult += json_object_set_new(output, "motorWhtRly", json(motorWhtRly));
	packResult += json_object_set_new(output, "motorYlwRly", json(motorYlwRly));
	packResult += json_object_set_new(output, "motorRedRly", json(motorRedRly));
	packResult += json_object_set_new(output, "motorRedWhtRly", json(motorRedWhtRly));
	packResult += json_object_set_new(output, "motorRedYlwRly", json(motorRedYlwRly));
	packResult += json_object_set_new(output, "servoPower", json(servoPower));
	packResult += json_object_set_new(output, "startStopTime", json_integer(startStopTime));
	packResult += json_object_set_new(output, "startModeTime", json_integer(startModeTime));
	packResult += json_object_set_new(output, "originMode", json(originMode));
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

bool arduinoStateClass::parse (json_t *input, bool seq = true) {
	json_t *tmp;
	if (seq) {
		tmp = json_object_get(input, "sequenceNum");
		if (!json_is_integer(tmp))
			return false;
		_sequenceNum = json_integer_value(tmp);
	}

	Mode tmp_command = arduinoModeEnum::NONE;
	boneStateClass::Mode tmp_boat = boatModeEnum::NONE;
	double tmp_float;

	::parse(json_object_get(input, "popStatus"), &popStatus);
	::parse(json_object_get(input, "mode"), &mode);
	::parse(json_object_get(input, "command"), &tmp_command);
	::parse(json_object_get(input, "boat"), &tmp_boat);
	setCommand(tmp_command);
	setBoatMode(tmp_boat);

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
			printf("(%zd)%s;", bytesRead, buf);
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
		usleep(10000);
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
	struct termios2 ard_attrib;
	
	ard_fd = open(ARDUINO_REST_TTY, O_RDWR | O_NONBLOCK | O_NOCTTY);
	//ard_fd = open(ARDUINO_REST_TTY, O_RDWR | O_NOCTTY);
	if (ard_fd == -1) return ard_fd;	
	if (ioctl(ard_fd, TCGETS2, &ard_attrib) < 0) {
		errLog->write("Arduino Serial", "Unable to get serial properties");
		closeArduinoSerial();
		return ard_fd;
	}
	//cfsetspeed(&ard_attrib, B115200);
	//cfsetspeed(&ard_attrib, B230400);
	//cfmakeraw(&ard_attrib);
	ard_attrib.c_cflag &= ~CBAUD;
	ard_attrib.c_cflag |= BOTHER;
	ard_attrib.c_ispeed = 1000000;
	ard_attrib.c_ospeed = 1000000;
	ard_attrib.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	ard_attrib.c_cflag &= ~(PARENB | CSTOPB | CSIZE);		// no parity, one stop bit
	ard_attrib.c_cflag |= (CLOCAL | CREAD | CS8);			// ignore modem status lines, enable receiver, 8 bits per byte
	ard_attrib.c_iflag &= ~(IXON | IXOFF | IXANY);			// turn off all flow control
	ard_attrib.c_iflag &= ~(ICRNL);							// turn off line ending translation					
	ard_attrib.c_oflag &= ~(OPOST);							// turn off post processing of output
	ard_attrib.c_cc[VMIN] = 0;								// this sets the timeouts for the read() operation to minimum
	ard_attrib.c_cc[VTIME] = 1;
	if (ioctl(ard_fd, TCSETS2, &ard_attrib) < 0) {
		closeArduinoSerial();
		errLog->write("Arduino Serial", "Unable to set serial properties");
		return ard_fd;
	}
	
	return ard_fd;
}

void arduinoStateClass::closeArduinoSerial (void) {
	//tcflush(ard_fd, TCIOFLUSH);
	close(ard_fd);
	ard_fd = -1;
}
