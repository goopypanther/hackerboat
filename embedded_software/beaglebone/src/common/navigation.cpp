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

navVectorClass::navVectorClass (string src, double bearing, double strength) {
	_source = src;
	_bearing = bearing;
	_strength = strength;
}

bool inline navVectorClass::isValid (void) const {
	if ((_bearing >= 0) && (_bearing <= 360) && 
		(_strength >= 0) && isnormal(_bearing) &&
		isnormal(_strength)) return true;
	return false;
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

navVectorClass navVectorClass::add (navVectorClass a) {
	// formulas from http://math.stackexchange.com/questions/1365622/adding-two-polar-vectors
	navVectorClass out;
	if ((!a.isValid()) && (!this->isValid())) return out;
	double deltaBearing;
	double r1sq, r2sq, r1r2;
	deltaBearing = locationClass::deg2rad(a._bearing - this->_bearing);
	r1sq = pow(this->_strength, 2);
	r1sq = pow(a._strength, 2);
	r1r2 = a._strength * this->_strength;
	out._strength = sqrt(r1sq + r2sq + (2 * r1r2 * cos(deltaBearing)));
	out._bearing = this->_bearing + locationClass::rad2deg(acos((this->_strength + (a._strength * cos(deltaBearing)))/out._strength)); 
	return out;
}

bool navVectorClass::parse(json_t *input,  bool seq) {
	char buf[LOCAL_BUF_LEN];
	if (json_unpack(input, this->_format.c_str(), "source", buf, "bearing", &_bearing, "strength", &_strength)) {
		return false;
	}
	this->_source.assign(buf, LOCAL_BUF_LEN);
	return this->isValid();	
}

json_t* navVectorClass::pack(bool seq) {
	return json_pack(this->_format.c_str(), "source", this->_source.c_str(), "bearing", _bearing, "strength", _strength));
}

bool navClass::parse(json_t *input,  bool seq) {
	json_t *navArrayIn, *currentIn, *targetIn, *targetVecIn, *totalIn, *seqIn;
	if (json_unpack(input, this->_formatFile.c_str(), 
					"current", currentIn,
					"target", targetIn,
					"waypointStrength", &waypointStrength,
					"magCorrection", &magCorrection,
					"targetVec", targetVecIn,
					"total", totalIn,
					"navInfluences", navArrayIn)) {
		return false;
	}
	
	if (seq) {
		seqIn = json_object_get(input, "sequenceNum");
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}
	current.parse(currentIn));
	target.parse(targetIn));
	targetVec.parse(targetVecIn));
	total.parse(totalIn));
	int32_t influenceCount = json_array_size(navArrayIn);
	for (uint16_t i; i < influenceCount; i++) {
		locationClass tmp;
		tmp.parse(json_array_get(navArrayIn, i));
		navInfluences.push_back(tmp);
	}
	free(navArrayIn);
	free(currentIn);
	free(targetIn);
	free(targetVecIn);
	free(totalIn);
	return this->isValid();
}

json_t* navClass::pack (bool seq) {
	json_t *array, *output;
	array = json_array();
	for (uint16_t i; i < navInfluences.size(); i++) {
		json_array_append_new(array, navInfluences.at(i).pack());
	}
	output = json_pack(this->_format.c_str(),  
						"current", current.pack(),
						"target", target.pack(),
						"waypointStrength", waypointStrength,
						"magCorrection", magCorrection,
						"targetVec", targetVec.pack(),
						"total", total.pack(),
						"navInfluences", array);
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool navClass::parseInfluences(json_t *navArrayIn) {
	if (!json_is_array(navArrayIn))
	    return false;
	int influenceCount = json_array_size(navArrayIn);
	navInfluences.resize(influenceCount);
	for (int i; i < influenceCount; i++) {
		if (!navInfluences[i].parse(json_array_get(navArrayIn, i)))
			return false;
	}
}

json_t *navClass::packInfluences(void) const {
	json_t *array = json_array();
	for (auto it = navInfluences.cbegin(); it != navInfluences.cend(); it ++) {
		json_array_append_new(array, it->pack());
	}
	return array;
}

hackerboatStateStorage &navClass::storage() {
	static hackerboatStateStorage *navStorage;

	if (!navStorage) {
		navStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(":memory:"),
							"NAV",
							{ { "current", "TEXT"    },
							  { "target",  "INTEGER" },
							  { "total",   "TEXT"    },
							  { "other",   "TEXT"    } });
		navStorage->createTable();
	}

	return *navStorage;
}

bool navClass::fillRow(sqliteParameterSlice row) const
{
	row.assertWidth(4);

	row.bind_json_new(0, current.pack());
	row.bind(1, target.getSequenceNum());
	row.bind_json_new(2, total.pack());

	json_t *array = json_array();
	for (int i = 0; i < navInfluences.size(); i++) {
		json_array_append_new(array, navInfluences[i].pack());
	}
	row.bind_json_new(3, json_pack("{s:f,s:f,s:o,s:o}",
				       "waypointStrength", waypointStrength,
				       "magCorrection", magCorrection,
				       "targetVec", targetVec.pack(),
				       "navInfluences", array));

	return true;
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

	sequence foo = row.int64_field(1);
#warning Wim - implement simple foreign key support, or change class interface

	repr = row.json_field(2);
	if (!total.parse(repr))
		success = false;
	json_decref(repr);

	repr = row.json_field(3);
	if (json_unpack(repr, "{s:f,s:f,s:o,s:o}",
			"waypointStrength", &waypointStrength,
			"magCorrection", &magCorrection,
			"targetVec", &targetVecIn,
			"navInfluences", &influences)) {
		success = false;
	}

	if (!targetVec.parse(targetVecIn))
		success = false;
	if (!parseInfluences(influences))
		success = false;
	json_decref(repr);

	return success;
}

bool navClass::appendVector (navVectorClass vec) {
	if (vec.isValid()) {
		this->navInfluences.push_back(vec);
		return true;
	} else return false;
}

bool navClass::calc (double maxStrength) {
	targetVec._bearing = this->current.bearing(target.location, locationClass::RhumbLine);
	targetVec._strength = waypointStrength;
	if (targetVec.isValid()) {
		total = targetVec;
	} else return false;
	for (uint16_t i; i < this->navInfluences.size(); i++) {
		if (navInfluences.at(i).isValid()) {
			total = total.add(navInfluences.at(i));
		} 
		if (!total.isValid()) return false;
	}
	if (total._strength > maxStrength) total.strength = maxStrength;
	return true;
}

void navClass::clearVectors (void) {
	this->navInfluences.clear();
}

bool navClass::isValid (void) const {
	bool out = false;
	if (current.isValid() && target.isValid() &&
		targetVec.isValid() && total.isValid() &&
		isnormal(waypointStrength) && isnormal(magCorrection)) {
		out = true;
	} else out = false;
	if (out && this->navInfluences.size() > 0) {
		for (uint16_t i; i < navInfluences.size(); i++) {
			out &= navInfluences.at(i).isValid();
		}
	}
	return out;
}