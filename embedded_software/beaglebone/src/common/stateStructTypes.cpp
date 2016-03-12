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
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"

#include <string>
using namespace std;

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
