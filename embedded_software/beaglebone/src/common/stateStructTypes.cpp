/******************************************************************************
 * Hackerboat Beaglebone types module
 * stateStructTypes.cpp
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <array>
#include <string>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"

json_t *hackerboatStateClass::packTimeSpec (timespec t) {
	return json_pack("{s:i,s:i}", "tv_sec", t.tv_sec, "tv_nsec", t.tv_nsec);
}

int hackerboatStateClass::parseTimeSpec (json_t *input, timespec *t) {
	return json_unpack(input, "{s:i,s:i}", "tv_sec", &(t->tv_sec), "tv_nsec", &(t->tv_nsec));
}

json_t *orientationClass::pack (bool seq) {
	json_t *output = json_pack(this->_format.c_str(),
								"roll", roll,
								"pitch", pitch,
								"yaw", yaw);
	return output;
}

bool orientationClass::parse (json_t *input, bool seq = true) {
	if (json_unpack(input, this->_format.c_str(),					
					"roll", roll,
					"pitch", pitch,
					"yaw", yaw)) {
		return false;
	}
	return this->isValid();
}

bool orientationClass::isValid (void) {
	return this->normalize();
}

bool orientationClass::normalize (void) {
	bool result = true;
	if (isfinite(roll)) {roll = fmod(roll, maxVal);} else {result = false;}
	if (isfinite(pitch)) {pitch = fmod(pitch, maxVal);} else {result = false;}
	if (isfinite(yaw)) {yaw = fmod(yaw, maxVal);} else {result = false;}
	return result;
}



json_t *waypointClass::pack (bool seq) {
	json_t *output;
	output = json_pack(this->_format.c_str(),
						"location", location.pack(), 
						"index", index,
						"act", act);
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool waypointClass::parse (json_t *input, bool seq = true) {
	json_t *inLoc, *seqIn;
	if (json_unpack(input, this->_format.c_str(),
					"location", inLoc, 
					"index", &index,
					"act", &act)) {
		return false;
	}
	location.parse(inLoc);
	if (seq) {
		seqIn = json_object_get(input, "sequenceNum");
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}
	free(seqIn);
	free(inLoc);
	return this->isValid();	
}

bool waypointClass::isValid (void) {
	if ((act < minActionEnum) || (act > maxActionEnum)) return false;
	return location.isValid();
}

json_t *boneStateClass::pack (bool seq) {
	json_t *output;
	output = json_pack(this->_format.c_str(),
						"uTime", packTimeSpec(uTime),
						"lastContact", packTimeSpec(lastContact),
						"state", state,
						"stateString", json_string(stateString.c_str()),
						"command", command,
						"commandString", json_string(commandString.c_str()),
						"ardState", ardState,
						"ardStateString", json_string(ardStateString.c_str()),
						"faultString", json_string(faultString.c_str()),
						"gps", gps.pack(),
						"waypointNext", waypointNext,
						"waypointStrength", waypointStrength,
						"waypointAccuracy", waypointAccuracy,
						"waypointStrengthMax", waypointStrengthMax,
						"autonomous", autonomous,
						"launchPoint", launchPoint.pack());
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool boneStateClass::parse (json_t *input, bool seq = true) {
	json_t *seqIn, *inGNSS, *inUtime, *inLastContact, *inLaunch;
	char inStateString[LOCAL_BUF_LEN], inCommandString[LOCAL_BUF_LEN];
	char inArdStateString[LOCAL_BUF_LEN], inFaultString[LOCAL_BUF_LEN];
	if (json_unpack(input, this->_format.c_str(),
						"uTime", inUtime,
						"lastContact", inLastContact,
						"state", &state,
						"stateString", inStateString,
						"command", &command,
						"commandString", inCommandString,
						"ardState", &ardState,
						"ardStateString", inArdStateString,
						"faultString", inFaultString,
						"gps", inGNSS,
						"waypointNext", &waypointNext,
						"waypointStrength", &waypointStrength,
						"waypointAccuracy", &waypointAccuracy,
						"waypointStrengthMax", &waypointStrengthMax,
						"autonomous", &autonomous,
						"launchPoint", inLaunch)) {
		return false;
	}
	if (seq) {
		seqIn = json_object_get(input, "sequenceNum");
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}
	parseTimeSpec(inUtime, &uTime);
	parseTimeSpec(inLastContact, &lastContact);
	gps.parse(inGNSS);
	launchPoint.parse(inLaunch);
	setState(state);
	setCommand(command);
	setArduinoState(ardState);
	faultString = std::string(inFaultString);
	free(seqIn);
	free(inUtime);
	free(inGNSS);
	free(inLastContact);
	free(inLaunch);
	return this->isValid();	
}

bool boneStateClass::isValid (void) {
	if ((state > boneStateCount) || (state < 0)) return false;
	if (boneStates[state] != stateString) return false;
	if ((command > boneStateCount) || (command < 0)) return false;
	if (boneStates[command] != commandString) return false;
	if ((ardState > arduinoStateClass::arduinoStateCount) || (ardState < 0)) return false;
	if (arduinoStateClass::arduinoStates[ardState] != ardStateString) return false;
	if (!gps.isValid()) return false;
	if (!launchPoint.isValid()) return false;
	if (waypointStrength < 0) return false;
	if (waypointAccuracy < 0) return false;
	if (waypointStrengthMax < 0) return false;
	return true;
}

json_t *arduinoStateClass::pack (bool seq) {
	json_t *output = json_object();
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	json_object_set(output, "popState", json_boolean(popStatus));
	json_object_set(output, "uTime", packTimeSpec(uTime));
	json_object_set(output, "state", json_integer(state));
	json_object_set(output, "command", json_integer(command));
	json_object_set(output, "throttle", json_integer(throttle));
	json_object_set(output, "bone", json_integer(bone));
	json_object_set(output, "orientation", orientation.pack());
	json_object_set(output, "headingTarget", json_real(headingTarget));
	json_object_set(output, "internalVoltage", json_real(internalVoltage));
	json_object_set(output, "batteryVoltage", json_real(batteryVoltage));
	json_object_set(output, "motorVoltage", json_real(motorVoltage));
	json_object_set(output, "enbButton", json_boolean(enbButton));
	json_object_set(output, "stopButton", json_boolean(stopButton));
	json_object_set(output, "timeSinceLastPacket", json_integer(timeSinceLastPacket));
	json_object_set(output, "timeOfLastPacket", json_integer(timeOfLastPacket));
	json_object_set(output, "timeOfLastBoneHB", json_integer(timeOfLastBoneHB));
	json_object_set(output, "timeOfLastShoreHB", json_integer(timeOfLastShoreHB));
	json_object_set(output, "stateString", json_string(stateString.c_str()));
	json_object_set(output, "boneStateString", json_string(boneStateString.c_str()));
	json_object_set(output, "commandString", json_string(commandString.c_str()));
	json_object_set(output, "faultString", json_string(faultString.c_str()));
	json_object_set(output, "rudder", json_real(rudder));
	json_object_set(output, "rudderRaw", json_integer(rudderRaw));
	json_object_set(output, "internalVoltageRaw", json_integer(internalVoltageRaw));
	json_object_set(output, "motorVoltageRaw", json_integer(motorVoltageRaw));
	json_object_set(output, "motorCurrent", json_real(motorCurrent));
	json_object_set(output, "motorCurrentRaw", json_integer(motorCurrent));
	json_object_set(output, "Kp", json_real(Kp));
	json_object_set(output, "Ki", json_real(Ki));
	json_object_set(output, "Kd", json_real(Kd));
	json_object_set(output, "magX", json_real(magX));
	json_object_set(output, "magY", json_real(magY));
	json_object_set(output, "magZ", json_real(magZ));
	json_object_set(output, "gyroX", json_real(gyroX));
	json_object_set(output, "gyroY", json_real(gyroY));
	json_object_set(output, "gyroZ", json_real(gyroZ));
	json_object_set(output, "horn", json_boolean(horn));
	json_object_set(output, "motorDirRly", json_boolean(motorDirRly));
	json_object_set(output, "motorWhtRly", json_boolean(motorWhtRly));
	json_object_set(output, "motorYlwRly", json_boolean(motorYlwRly));
	json_object_set(output, "motorRedRly", json_boolean(motorRedRly));
	json_object_set(output, "motorRedWhtRly", json_boolean(motorRedWhtRly));
	json_object_set(output, "motorRedYlwRly", json_boolean(motorRedYlwRly));
	json_object_set(output, "servoPower", json_boolean(servoPower));
	json_object_set(output, "startStopTime", json_integer(startStopTime));
	json_object_set(output, "startStateTime", json_integer(startStateTime));
	json_object_set(output, "originState", json_integer(originState));
	return output;
}

bool arduinoStateClass::parse (json_t *input, bool seq = true) {
	json_t *tmp;
	if (seq) {
		tmp = json_object_get(input, "sequenceNum");
		if (tmp) _sequenceNum = json_integer_value(tmp);
		free(tmp);
	}
	tmp = json_object_get(input, "popStatus");
	if (tmp) popStatus = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "uTime");
	if (tmp) parseTimeSpec(tmp, &uTime);
	free(tmp);
	tmp = json_object_get(input, "state");
	if (tmp) {
		state = (arduinoStateEnum)json_integer_value(tmp);
		setState(state);
	}
	free(tmp);
	tmp = json_object_get(input, "command");
	if (tmp) {
		command = (arduinoStateEnum)json_integer_value(tmp);
		setCommand(command);
	}
	free(tmp);
	tmp = json_object_get(input, "throttle");
	if (tmp) throttle = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "bone");
	if (tmp) {
		bone = (boneStateClass::boneStateEnum)json_integer_value(tmp);
		setBoneState(bone);
	}
	free(tmp);
	tmp = json_object_get(input, "orientation");
	if (tmp) orientation.parse(tmp);
	free(tmp);
	tmp = json_object_get(input, "headingTarget");
	if (tmp) headingTarget = fmod(360.0, json_real_value(tmp));
	free(tmp);
	tmp = json_object_get(input, "internalVoltage");
	if (tmp) internalVoltage = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "batteryVoltage");
	if (tmp) batteryVoltage = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorVoltage");
	if (tmp) motorVoltage = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "enbButton");
	if (tmp) enbButton = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "stopButton");
	if (tmp) stopButton = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "timeSinceLastPacket");
	if (tmp) timeSinceLastPacket = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "timeOfLastPacket");
	if (tmp) timeOfLastPacket = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "timeOfLastBoneHB");
	if (tmp) timeOfLastBoneHB = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "timeOfLastShoreHB");
	if (tmp) timeOfLastShoreHB = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "faultString");
	if (tmp) faultString = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "rudder");
	if (tmp) rudder = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "rudderRaw");
	if (tmp) rudderRaw = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "internalVoltageRaw");
	if (tmp) internalVoltageRaw = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorVoltageRaw");
	if (tmp) motorVoltageRaw = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorCurrent");
	if (tmp) motorCurrent = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorCurrentRaw");
	if (tmp) motorCurrentRaw = json_integer_value(tmp);
	free(tmp);
	
	// rudder PID constants
	tmp = json_object_get(input, "Kp");
	if (tmp) Kp = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "Ki");
	if (tmp) Ki = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "Kd");
	if (tmp) Kd = json_real_value(tmp);
	free(tmp);
	
	// magnetometer data
	tmp = json_object_get(input, "magX");
	if (tmp) magX = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "magY");
	if (tmp) magY = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "magZ");
	if (tmp) magZ = json_real_value(tmp);
	free(tmp);
	
	// acceleration data
	tmp = json_object_get(input, "accX");
	if (tmp) accX = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "accY");
	if (tmp) accY = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "accZ");
	if (tmp) accZ = json_real_value(tmp);
	free(tmp);
	
	// gyro data
	tmp = json_object_get(input, "gyroX");
	if (tmp) gyroX = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "gyroY");
	if (tmp) gyroY = json_real_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "gyroZ");
	if (tmp) gyroZ = json_real_value(tmp);
	free(tmp);
	
	// relay states
	tmp = json_object_get(input, "horn");
	if (tmp) horn = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorDirRly");
	if (tmp) motorDirRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorWhtRly");
	if (tmp) motorWhtRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorYlwRly");
	if (tmp) motorYlwRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorRedRly");
	if (tmp) motorRedRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorRedWhtRlyorn");
	if (tmp) motorRedWhtRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "motorRedYlwRly");
	if (tmp) motorRedYlwRly = json_boolean_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "servoPower");
	if (tmp) servoPower = json_boolean_value(tmp);
	free(tmp);
	
	// state timers
	tmp = json_object_get(input, "startStopTime");
	if (tmp) startStopTime = json_integer_value(tmp);
	free(tmp);
	tmp = json_object_get(input, "startStateTime");
	if (tmp) startStateTime = json_integer_value(tmp);
	free(tmp);
	
	tmp = json_object_get(input, "originState");
	if (tmp) originState = (arduinoStateEnum)json_integer_value(tmp);
	free(tmp);
	
	return this->isValid();
}
	