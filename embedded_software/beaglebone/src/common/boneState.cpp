/******************************************************************************
 * Hackerboat Beaglebone types module
 * boneState.cpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
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
#include "arduinoState.hpp"
#include "boneState.hpp"

#include <string>
using namespace std;
 
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

bool boneStateClass::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		faultString += ":" + fault;
	}
	return true;
}

bool boneStateClass::hasFault (const string fault) {
	if (faultString.find(fault) != std::basic_string::npos) return true;
	return false;
}

bool boneStateClass::removeFault (const string fault) {
	size_t index, colon;
	index = faultString.find(fault);
	if (index != std::basic_string::npos) {
		faultString.erase(index, fault.length());
		colon = faultString.find(':', index);
		if (colon != std::basic_string::npos) {
			faultString.erase(colon, 1);
		}
		return true;
	} else return false;
}

int boneStateClass::failCount (void) {
	size_t index = faultString.find(':');
	int cnt = 0;
	while (index != std::basic_string::npos) {
		cnt++;
		index = faultString.find(':', index);
	}
	return cnt;
}
