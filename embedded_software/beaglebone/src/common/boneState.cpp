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

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

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
	json_t *output = json_object();
	int packResult = 0;
	if (seq) json_object_set_new(output, "sequenceNum", json_integer(_sequenceNum));
	packResult += json_object_set_new(output, "uTime", packTimeSpec(uTime));
	packResult += json_object_set_new(output, "lastContact", packTimeSpec(lastContact));
	packResult += json_object_set_new(output, "mode", json(mode));
	packResult += json_object_set_new(output, "command", json(command));
	packResult += json_object_set_new(output, "ardMode", json(ardMode));
	packResult += json_object_set_new(output, "faultString", json(faultString));
	packResult += json_object_set_new(output, "gps", gps.pack(seq));
	packResult += json_object_set_new(output, "waypointNext", json_integer(waypointNext));
	packResult += json_object_set_new(output, "waypointStrength", json_real(waypointStrength));
	packResult += json_object_set_new(output, "waypointAccuracy", json_real(waypointAccuracy));
	packResult += json_object_set_new(output, "waypointStrengthMax", json_real(waypointStrengthMax));
	packResult += json_object_set_new(output, "autonomous", json_boolean(autonomous));
	packResult += json_object_set_new(output, "launchPoint", launchPoint.pack());
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

bool boneStateClass::parse (json_t *input, bool seq = true) {
	json_t *inGNSS, *inUtime, *inLastContact, *inMode, *inCommand, *inArdMode, *inFaultString, *inLaunch;
	inUtime = json_object_get(input, "uTime");
	inLastContact = json_object_get(input, "lastContact");
	inMode = json_object_get(input, "mode");
	inCommand = json_object_get(input, "command");
	inArdMode = json_object_get(input, "ardMode");
	inFaultString = json_object_get(input, "faultString");
	inGNSS = json_object_get(input, "gps");
	GET_VAR(waypointNext);
	GET_VAR(waypointStrength);
	GET_VAR(waypointAccuracy);
	GET_VAR(waypointStrengthMax);
	GET_VAR(autonomous);
	inLaunch = json_object_get(input, "launchPoint");
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
		index = faultString.find(':', (index + 1));
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
		

