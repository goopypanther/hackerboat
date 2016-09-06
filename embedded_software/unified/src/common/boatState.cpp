/******************************************************************************
 * Hackerboat Beaglebone boat state module
 * boatState.cpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "location.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"
#include "hal/config.h"
#include "logs.hpp"
#include "enumdefs.hpp"
#include "healthMonitor.hpp"
#include "waypoint.hpp"
#include "dodge.hpp"
#include "hal/relay.hpp"
#include "hal/gpio.hpp"
#include "hal/adcInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/gpsdInput.hpp"
#include "boatState.hpp"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

BoatState::BoatState () {
	relays = RelayMap::instance();
}

bool BoatState::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		faultString += fault + ":";
	}
	return true;
}

bool BoatState::hasFault (const string fault) const {
	if (faultString.find(fault) != std::string::npos) return true;
	return false;
}

bool BoatState::removeFault (const string fault) {
	size_t index;
	index = faultString.find(fault);
	if (index != std::string::npos) {
		faultString.erase(index, fault.length() + 1);	// captures the trailing colon
		return true;
	} else return false;
}

int BoatState::faultCount (void) const {
	size_t index = faultString.find(':');
	int cnt = 0;
	while (index != std::string::npos) {
		cnt++;
		index = faultString.find(':', (index + 1));
	}
	return cnt;
}

bool BoatState::setBoatMode (std::string mode) {
	BoatModeEnum m;
	if (!boatModeNames.get(mode, &m)) {
		return false;
	}
	_boat = m;
	return true;
}

bool BoatState::setNavMode (std::string mode) {
	NavModeEnum m;
	if (!navModeNames.get(mode, &m)) {
		return false;
	}
	_nav = m;
	return true;
}
						
bool BoatState::setAutoMode (std::string mode) {
	AutoModeEnum m;
	if (!autoModeNames.get(mode, &m)) {
		return false;
	}
	_auto = m;
	return true;
}
						
bool BoatState::setRCmode (std::string mode) {
	RCModeEnum m;
	if (!rcModeNames.get(mode, &m)) {
		return false;
	}
	_rc = m;
	return true;
}

bool BoatState::isValid() {
	bool result = true;
	result &= lastFix.isValid();
	result &= launchPoint.isValid();
	result &= (health && health->isValid());
	return result;
}

json_t* BoatState::pack () const {
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "recordTime", json(HackerboatState::packTime(this->recordTime)));
	packResult += json_object_set_new(output, "currentWaypoint", json_integer(currentWaypoint));
	packResult += json_object_set_new(output, "waypointStrength", json_real(waypointStrength));
	packResult += json_object_set_new(output, "lastContact", json(HackerboatState::packTime(this->lastContact)));
	packResult += json_object_set_new(output, "lastRC", json(HackerboatState::packTime(this->lastRC)));
	packResult += json_object_set_new(output, "lastFix", this->lastFix.pack());
	packResult += json_object_set_new(output, "launchPoint", this->launchPoint.pack());
	packResult += json_object_set_new(output, "action", json(Waypoints::actionNames.get(action)));
	packResult += json_object_set_new(output, "faultString", json(this->faultString));
	packResult += json_object_set_new(output, "boatMode", json(boatModeNames.get(_boat)));
	packResult += json_object_set_new(output, "navMode", json(navModeNames.get(_nav)));
	packResult += json_object_set_new(output, "autoMode", json(autoModeNames.get(_auto)));
	packResult += json_object_set_new(output, "rcMode", json(rcModeNames.get(_rc)));
	packResult += json_object_set_new(output, "relays", this->relays->pack());
	
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool BoatState::parse (json_t* input ) {
	std::string recordTimeIn, lastContactIn, lastRCin, boatMode, navMode, autoMode, rcMode;
	bool result = true;
	
	result &= GET_VAR(currentWaypoint);
	result &= GET_VAR(waypointStrength);
	result &= GET_VAR(faultString);
	result &= GET_VAR(boatMode);
	result &= GET_VAR(navMode);
	result &= GET_VAR(autoMode);
	result &= GET_VAR(rcMode);
	result &= ::parse(json_object_get(input, "recordTime"), &recordTimeIn);
	result &= ::parse(json_object_get(input, "lastContact"), &lastContactIn);
	result &= ::parse(json_object_get(input, "lastRC"), &lastRCin);
	result &= HackerboatState::parseTime(recordTimeIn, this->recordTime);
	result &= HackerboatState::parseTime(lastContactIn, this->lastContact);
	result &= HackerboatState::parseTime(lastRCin, this->lastRC);
	result &= boatModeNames.get(boatMode, &(this->_boat));
	result &= navModeNames.get(boatMode, &(this->_nav));
	result &= autoModeNames.get(boatMode, &(this->_auto));
	result &= rcModeNames.get(boatMode, &(this->_rc));
	
	return result;
}

HackerboatStateStorage &BoatState::storage () {
	if (!stateStorage) {
		stateStorage = new HackerboatStateStorage(HackerboatStateStorage::databaseConnection(STATE_DB_FILE), 
							"BOAT_STATE",
							{ { "recordTime", "TEXT" },
							  { "currentWaypoint", "INTEGER" },
							  { "waypointStrength", "REAL" },
							  { "lastContact", "TEXT" },
							  { "lastRC", "TEXT" },
							  { "lastFix", "TEXT" },
							  { "launchPoint", "TEXT" },
							  { "action", "TEXT" },
							  { "faultString", "TEXT" },
							  { "boatMode", "TEXT" },
							  { "navMode", "TEXT" },
							  { "autoMode", "TEXT" },
							  { "rcMode", "TEXT" },
							  { "relays", "TEXT" } });
		stateStorage->createTable();						 
	}
	return *stateStorage;
}

bool BoatState::fillRow(SQLiteParameterSlice row) const {
	row.assertWidth(14);
	json_t* out;
	
	row.bind(0, HackerboatState::packTime(recordTime));
	row.bind(1, currentWaypoint);
	row.bind(2, waypointStrength);
	row.bind(3, HackerboatState::packTime(lastContact));
	row.bind(4, HackerboatState::packTime(lastRC));
	out = this->lastFix.pack();
	row.bind(5, json_dumps(out,0));
	json_decref(out);
	out  = this->launchPoint.pack();
	row.bind(6, json_dumps(out,0));
	json_decref(out);
	row.bind(7, Waypoints::actionNames.get(action));
	row.bind(8, faultString);
	row.bind(9, boatModeNames.get(_boat));
	row.bind(10, navModeNames.get(_nav));
	row.bind(11, autoModeNames.get(_auto));
	row.bind(12, rcModeNames.get(_rc));
	out = this->relays->pack();
	row.bind(13, json_dumps(out,0));
	json_decref(out);
	
	return true;
}

bool BoatState::readFromRow(SQLiteRowReference row, sequence seq) {
	bool result = true;
	_sequenceNum = seq;
	row.assertWidth(14);

	std::string str = row.string_field(0);
	result &= HackerboatState::parseTime(str, this->recordTime);
	this->currentWaypoint = row.int_field(1);
	this->waypointStrength = row.double_field(2);
	str = row.string_field(3);
	result &= HackerboatState::parseTime(str, this->lastContact);
	str = row.string_field(4);
	result &= HackerboatState::parseTime(str, this->lastRC);
	str = row.string_field(5);
	result &= this->lastFix.parse(json_loads(str.c_str(), str.size(), NULL));
	str = row.string_field(6);
	result &= this->launchPoint.parse(json_loads(str.c_str(), str.size(), NULL));
	str = row.string_field(7);
	result &= Waypoints::actionNames.get(str, &(this->action));
	this->faultString = row.string_field(8);
	str = row.string_field(9);
	result &= boatModeNames.get(str, &(this->_boat));
	str = row.string_field(10);
	result &= navModeNames.get(str, &(this->_nav));
	str = row.string_field(11);
	result &= autoModeNames.get(str, &(this->_auto));
	str = row.string_field(12);
	result &= rcModeNames.get(str, &(this->_rc));
	
	return result;
}
