/******************************************************************************
 * Hackerboat Beaglebone navigation module
 * navigation.cpp
 * This module stores and processes navigation data
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <array>
#include "config.h"
#include "stateStructTypes.hpp"
#include "location.hpp"
#include "navigation.hpp"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

navVectorClass::navVectorClass (std::string src, double bearing, double strength)
  : _source(src), _bearing(bearing), _strength(strength)
{
}

bool navVectorClass::isValid (void) const {
	if (!isfinite(_bearing) || !((_bearing >= 0) && (_bearing <= 360)))
		return false;
	if (!isfinite(_strength) || !(_strength >= 0))
		return false;
	return true;
}

bool navVectorClass::norm (void) {
	if (isnormal(_strength) && isnormal(_bearing)) {
		while (!this->isValid()) {
			_strength = abs(_strength);
			if (_bearing > 360) {
				_bearing -= 360;
			} else if (_bearing < 0) {
				_bearing += 360;
			}
		}
		return true;
	} else return false;
}

navVectorClass navVectorClass::add (const navVectorClass& a) const {
	// formulas from http://math.stackexchange.com/questions/1365622/adding-two-polar-vectors
	navVectorClass out;
	if ((!a.isValid()) && (!this->isValid())) return out;
	double deltaBearing;
	double r1sq, r2sq, r1r2;
	deltaBearing = locationClass::deg2rad(a._bearing - this->_bearing);
	r1sq = pow(this->_strength, 2);
	r2sq = pow(a._strength, 2);
	r1r2 = a._strength * this->_strength;
	out._strength = sqrt(r1sq + r2sq + (2 * r1r2 * cos(deltaBearing)));
	out._bearing = this->_bearing + locationClass::rad2deg(acos((this->_strength + (a._strength * cos(deltaBearing)))/out._strength)); 
	return out;
}

bool navVectorClass::parse(json_t *input) {
	json_t *buf;
	buf = json_object_get(input, "source");
	if (!::parse(json_object_get(input, "bearing"), &_bearing) ||
		!::parse(json_object_get(input, "strength"), &_strength)) {
		return false;
	}
	if (!json_is_string(buf))
		return false;
	this->_source.assign(json_string_value(buf), json_string_length(buf));
	return this->isValid();
}

json_t* navVectorClass::pack(void) const {
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "source", json(_source));
	packResult += json_object_set_new(output, "bearing", json_real(_bearing));
	packResult += json_object_set_new(output, "strength", json_real(_strength));
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

bool navClass::parse(json_t *input, bool seq) {
	json_t *navArrayIn, *currentIn, *targetIn, *targetVecIn, *totalIn;
	if (!::parse(json_object_get(input, "waypointStrength"), &waypointStrength) ||
		!::parse(json_object_get(input, "magCorrection"), &magCorrection)) {
		return false;
	}
	currentIn = json_object_get(input, "current");
	targetIn = json_object_get(input, "targetWaypoint");
	targetVecIn = json_object_get(input, "targetVec");
	totalIn = json_object_get(input, "total");
	navArrayIn = json_object_get(input,"navInfluences");
	
	if (seq) {
		json_t *seqIn = json_object_get(input, "sequenceNum");
		if (!json_is_integer(seqIn))
			return false;
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}

	if (!current.parse(currentIn))
		return false;
	if (!::parse(targetIn, &targetWaypoint))
		return false;
	if (!targetVec.parse(targetVecIn))
		return false;
	if (!total.parse(totalIn))
		return false;
	if (!parseInfluences(navArrayIn))
		return false;
	/* We asked json_unpack() to give us borrowed references for all of these values, so we don't need to decref them here. */
	return this->isValid();
}

json_t* navClass::pack (bool seq) const {
	json_t *output = json_object();
	int packResult = 0;
	if (seq) json_object_set_new(output, "sequenceNum", json_integer(_sequenceNum));
	packResult += json_object_set_new(output, "current", current.pack());
	packResult += json_object_set_new(output, "targetWaypoint", json_integer(targetWaypoint));
	packResult += json_object_set_new(output, "waypointStrength", json_real(waypointStrength));
	packResult += json_object_set_new(output, "magCorrection", json_real(magCorrection));
	packResult += json_object_set_new(output, "targetVec", targetVec.pack());
	packResult += json_object_set_new(output, "total", total.pack());
	packResult += json_object_set_new(output, "navInfluences", packInfluences());
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

bool navClass::parseInfluences(json_t *navArrayIn) {
	if (!json_is_array(navArrayIn))
	    return false;
	size_t influenceCount = json_array_size(navArrayIn);
	navInfluences.resize(influenceCount);
	for (size_t i = 0; i < influenceCount; i++) {
		if (!navInfluences[i].parse(json_array_get(navArrayIn, i)))
			return false;
	}

	return true;
}

json_t *navClass::packInfluences(void) const {
	json_t *array = json_array();
	for (auto it = navInfluences.cbegin(); it != navInfluences.cend(); it ++) {
		json_t *entry = it->pack();
		if (!entry) {
			json_decref(array);
			return NULL;
		}
		json_array_append_new(array, entry);
	}
	return array;
}

hackerboatStateStorage &navClass::storage() {

	if (!navStorage) {
		navStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(NAV_DB_FILE),
							"NAV",
							{ { "json", "TEXT"    } });
		navStorage->createTable();
	}

	return *navStorage;
}

bool navClass::fillRow(sqliteParameterSlice row) const
{
	row.assertWidth(4);

	row.bind_json_new(0, current.pack());
	row.bind(1, targetWaypoint);
	row.bind_json_new(2, total.pack());

	json_t *array = json_array();
	for (typeof(navInfluences.size()) i = 0; i < navInfluences.size(); i++) {
		json_array_append_new(array, navInfluences[i].pack());
	}
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "waypointStrength", json_real(waypointStrength));
	packResult += json_object_set_new(output, "magCorrection", json_real(magCorrection)); 
	packResult += json_object_set_new(output, "targetVec", targetVec.pack());
	packResult += json_object_set_new(output, "navInfluences", array);
	if (packResult == 0) {
		row.bind_json_new(3, output);
		return true;
	} else {
		json_decref(output);
		return false;
	}
}

bool navClass::readFromRow(sqliteRowReference row, sequence assignedId)
{
	row.assertWidth(4);
	json_t *repr, *targetVecIn, *influences;

	bool success = true;

	_sequenceNum = assignedId;

	repr = row.json_field(0);
	if (!current.parse(repr))
		success = false;
	json_decref(repr);

	targetWaypoint = row.int64_field(1);

	repr = row.json_field(2);
	if (!total.parse(repr))
		success = false;
	json_decref(repr);

	repr = row.json_field(3);
	::parse(json_object_get(repr, "waypointStrength"), &waypointStrength);
	::parse(json_object_get(repr, "magCorrection"), &magCorrection);
	targetVecIn = json_object_get(repr, "targetVec");
	influences	= json_object_get(repr, "navInfluences");

	if (!targetVec.parse(targetVecIn))
		success = false;
	if (!parseInfluences(influences))
		success = false;
	json_decref(repr);

	return success;
}

bool navClass::appendVector (const navVectorClass& vec) {
	if (vec.isValid()) {
		this->navInfluences.push_back(vec);
		return true;
	} else {
		return false;
	}
}

bool navClass::calc (double maxStrength) {
	waypointClass target;
	if (!target.getRecord(targetWaypoint)) return false;
	targetVec._bearing = this->current.bearing(target.location, locationClass::RhumbLine);
	targetVec._strength = waypointStrength;
	if (!targetVec.isValid()) {
		return false;
	}
	total = targetVec;
	for (auto it = navInfluences.cbegin(); it != navInfluences.cend(); ++it) {
		if (it->isValid()) {
			total  = total.add(*it);
		}
		if (!total.isValid()) return false;
	}
	if (total._strength > maxStrength) total._strength = maxStrength;
	return true;
}

void navClass::clearVectors (void) {
	this->navInfluences.clear();
}

bool navClass::isValid (void) const {
	if (targetWaypoint < 0)
		return false;
	if (!current.isValid())
		return false;
	if (!targetVec.isValid())
		return false;
	if (!total.isValid())
		return false;
	if (!isnormal(waypointStrength) || !isnormal(magCorrection))
		return false;

	for (auto it = navInfluences.cbegin(); it != navInfluences.cend(); ++it) {
		if (!it->isValid()) {
			return false;
		}
	}

	return true;
}

bool navClass::initRecord(void) {
	return true;
}
