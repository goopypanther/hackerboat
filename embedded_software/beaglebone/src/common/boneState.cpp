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
#include "boneState.hpp"
#include "arduinoState.hpp"

#include <string>
using namespace std;
 
const enumerationNameTable<boatModeEnum> boneStateClass::modeNames = {
	"Start", 
	"SelfTest", 
	"Disarmed", 
	"Fault",
	"Armed", 
	"Manual", 
	"WaypointNavigation",
	"LossOfSignal", 
	"ReturnToLaunch", 
	"ArmedTest",
	"None"
};

json_t *boneStateClass::pack (bool seq) {
	json_t *output;
	output = json_pack(this->_format.c_str(),
						"uTime", packTimeSpec(uTime),
						"lastContact", packTimeSpec(lastContact),
						"mode", mode,
						"command", command,
						"ardMode", ardMode,
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
	if (json_unpack(input, this->_format.c_str(),
						"uTime", inUtime,
						"lastContact", inLastContact,
						"mode", &mode,
						"command", &command,
						"ardMode", &ardMode,
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
	setMode(mode);
	setCommand(command);
	setArduinoMode(ardMode);
	faultString = std::string(inFaultString);
	free(seqIn);
	free(inUtime);
	free(inGNSS);
	free(inLastContact);
	free(inLaunch);
	return this->isValid();	
}

bool boneStateClass::isValid (void) {
	if ((mode > boneStateCount) || (mode < 0)) return false;
	if ((command > boneStateCount) || (command < 0)) return false;
	if ((ardMode > arduinoStateClass::arduinoStateCount) || (ardMode < 0)) return false;
	if (!gps.isValid()) return false;
	if (!launchPoint.isValid()) return false;
	if (waypointStrength < 0) return false;
	if (waypointAccuracy < 0) return false;
	if (waypointStrengthMax < 0) return false;
	return true;
}

bool boneStateClass::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		faultString += fault + ":";
	}
	return true;
}

bool boneStateClass::hasFault (const string fault) {
	if (faultString.find(fault) != std::basic_string::npos) return true;
	return false;
}

bool boneStateClass::removeFault (const string fault) {
	size_t index;
	index = faultString.find(fault);
	if (index != std::string::npos) {
		faultString.erase(index, fault.length() + 1);	// captures the trailing colon
		return true;
	} else return false;
}

int boneStateClass::failCount (void) {
	size_t index = faultString.find(':');
	int cnt = 0;
	while (index != std::string::npos) {
		cnt++;
		index = faultString.find(':', index);
	}
	return cnt;
}

bool boneStateClass::setMode (boneModeEnum s) {
	if ((s > boneStateCount) || (s < 0)) return false;
	state = s;
	return true;
}

bool boneStateClass::setCommand (boneStateEnum c) {
	if ((c > boneStateCount) || (c < 0)) return false;
	command = c;
	return true;
}

bool boneStateClass::setArduinoMode (arduinoStateClass::Mode s) {
	if ((s > arduinoStateClass::arduinoStateCount) || (s < 0)) return false;
	ardState = s;
	return true;
}
		
void boneStateClass::initHashes (void) {
	for (int i = 0; i < boneStateCount; i++) {
		MurmurHash3_x86_32(boneStates[i].c_str(), boneStates[i].length(), HASHSEED, &(stateHashes[i]));
	}
}
