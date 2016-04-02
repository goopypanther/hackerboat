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
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include "config.h"
#include "json_utilities.hpp"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"
#include "boneState.hpp"
#include "arduinoState.hpp"
#include "sqliteStorage.hpp"

using string = std::string;

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

boneStateClass::boneStateClass() {
}

json_t *boneStateClass::pack (bool seq) const {
	json_t *output;
	clock_gettime(CLOCK_REALTIME, &uTime);
	output = json_pack("{s:o,s:o,s:o,s:o,s:o,s:o,s:o,s:i,s:f,s:f,s:f,s:b,s:o}",
			   "uTime", packTimeSpec(uTime),
			   "lastContact", packTimeSpec(lastContact),
			   "mode", json(mode),
			   "command", json(command),
			   "ardMode", json(ardMode),
			   "faultString", json(faultString),
			   "gps", gps.pack(seq),
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
	json_t *inGNSS, *inUtime, *inLastContact, *inMode, *inCommand, *inArdMode, *inFaultString, *inLaunch;
	if (json_unpack(input, "{s:o,s:o,s:o,s:o,s:o,s:o,s:o,s:i,s:F,s:F,s:F,s:b,s:o}",
						"uTime", &inUtime,
						"lastContact", &inLastContact,
						"mode", &inMode,
						"command", &inCommand,
						"ardMode", &inArdMode,
						"faultString", &inFaultString,
						"gps", &inGNSS,
						"waypointNext", &waypointNext,
						"waypointStrength", &waypointStrength,
						"waypointAccuracy", &waypointAccuracy,
						"waypointStrengthMax", &waypointStrengthMax,
						"autonomous", &autonomous,
						"launchPoint", &inLaunch)) {
		return false;
	}
	if (seq) {
		json_t *seqIn = json_object_get(input, "sequenceNum");
		if (!json_is_integer(seqIn))
			return false;
		_sequenceNum = json_integer_value(seqIn);
	}
	if (!::parse(inUtime, &uTime) ||
	    !::parse(inLastContact, &lastContact) ||
	    !::parse(inFaultString, &faultString) ||
	    !gps.parse(inGNSS, seq) ||
	    !launchPoint.parse(inLaunch))
		return false;
	{
		Mode tmp_mode;
		if (!::parse(inMode, &tmp_mode))
			return false;
		if (!setMode(tmp_mode))
			return false;
	}
	{
		Mode tmp_mode;
		if (!::parse(inCommand, &tmp_mode))
			return false;
		if (!setCommand(tmp_mode))
			return false;
	}
	{
		arduinoModeEnum tmp_mode;
		if (!::parse(inArdMode, &tmp_mode))
			return false;
		if (!setArduinoMode(tmp_mode))
			return false;
	}
	return this->isValid();	
}

bool boneStateClass::isValid (void) const {
	if (!modeNames.valid(mode)) return false;
	if (!modeNames.valid(command)) return false;
	if (!arduinoStateClass::modeNames.valid(ardMode)) return false;
	if (!gps.isValid()) return false;
	if (!launchPoint.isValid()) return false;
	if (waypointStrength < 0) return false;
	if (waypointAccuracy < 0) return false;
	if (waypointStrengthMax < 0) return false;
	return true;
}

hackerboatStateStorage &boneStateClass::storage() {
	static hackerboatStateStorage *boneStorage;

	if (!boneStorage) {
		boneStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(BONE_LOG_DB_FILE),
							    "BEAGLEBONE",
							    { { "json", "TEXT"    } });
		boneStorage->createTable();
	}

	return *boneStorage;
}

bool boneStateClass::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		faultString += fault + ":";
	}
	return true;
}

bool boneStateClass::hasFault (const string fault) const {
	if (faultString.find(fault) != std::string::npos) return true;
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

int boneStateClass::faultCount (void) const {
	size_t index = faultString.find(':');
	int cnt = 0;
	while (index != std::string::npos) {
		cnt++;
		index = faultString.find(':', index);
	}
	return cnt;
}

bool boneStateClass::setMode (boatModeEnum m) {
	if (!modeNames.valid(m))
		return false;
	mode = m;
	return true;
}

bool boneStateClass::setCommand (boatModeEnum m) {
	if (!modeNames.valid(m))
		return false;
	command = m;
	return true;
}

bool boneStateClass::setArduinoMode (arduinoStateClass::Mode s) {
	if (!arduinoStateClass::modeNames.valid(s))
		return false;
	ardMode = s;
	return true;
}
		

