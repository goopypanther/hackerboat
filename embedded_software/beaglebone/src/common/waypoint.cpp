/******************************************************************************
 * Hackerboat Beaglebone navigation module
 * waypoint.cpp
 * This module stores and processes navigation data
 * see the Hackerboat documentation for more details
 * 
 * Version 0.0: -
 *
 ******************************************************************************/

#include "stateStructTypes.hpp"

extern "C" {
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
}
#include <string>
#include "location.hpp"
#include "sqliteStorage.hpp"

waypointClass::waypointClass()
	: act(action::CONTINUE)
{
}

waypointClass::waypointClass (locationClass loc, action action)
	: location(loc), act(action)
{
}

bool waypointClass::setAction(waypointClass::action action)
{
	act = action;
	return true;
}

bool waypointClass::parse(json_t *input, bool seq)
{
	json_t *val;

	if (!location.parse(input))
		return false;

	val = json_object_get(input, "index");
	if (val) {
		if (!json_is_integer(val))
			return false;
		index = json_integer_value(val);
	} else
		index = -1;

	val = json_object_get(input, "nextWaypoint");
	if (val) {
		if (!json_is_integer(val))
			return false;
		nextWaypoint = json_integer_value(val);
	} else
		nextWaypoint = -1;

	val = json_object_get(input, "action");
	if (!::parse(val, &act))
		return false;

	if (seq) {
		json_t *seqIn = json_object_get(input, "sequenceNum");
		if (!seqIn)
			return false;
		_sequenceNum = json_integer_value(seqIn);
	}

	return this->isValid();
}

json_t *waypointClass::pack(bool seq) const
{
	json_t *repr = location.pack();

	if (index >= 0) {
		json_object_set_new_nocheck(repr, "index", json_integer(index));
	}
	if (nextWaypoint >= 0) {
		json_object_set_new_nocheck(repr, "nextWaypoint", json_integer(nextWaypoint));
	}
	json_object_set_new_nocheck(repr,
				    "action",
				    json_string(toString(act)));

	if (seq && (_sequenceNum >= 0)) {
		json_object_set_new_nocheck(repr, "sequenceNum", json_integer(_sequenceNum));
	}

	return repr;
}

bool waypointClass::isValid(void) const
{
	if (!location.isValid())
		return false;

	if (index < 0 || nextWaypoint < 0)
		return false;

	if (int(act) < 0 || int(act) > maxActionEnum)
		return false;

	return true;
}

const char *toString(enum waypointClass::action act)
{
	switch(act) {
	case waypointClass::action::CONTINUE:
		return "CONTINUE";
	case waypointClass::action::STOP:
		return "STOP";
	case waypointClass::action::HOME:
		return "HOME";
	}
	return NULL;
}

bool fromString(const char *name, waypointClass::action *act)
{
	if (!name)
		return false;
	/* World's simplest perfect hash function */
	waypointClass::action result;
	switch(name[0] % 3) {
	case 0: result = waypointClass::action::HOME; break;
	case 1: result = waypointClass::action::CONTINUE; break;
	case 2: result = waypointClass::action::STOP; break;
	default: return false;
	}
	if (!::strcmp(name, toString(result))) {
		*act = result;
		return true;
	} else {
		return false;
	}
}

bool waypointClass::fillRow(sqliteParameterSlice row) const
{
	row.assertWidth(5);

	if (!location.fillRow(row.slice(0, 2)))
		return false;

	if (index >= 0)
		row.bind(2, index);
	else
		row.bind_null(2);

	if (nextWaypoint >= 0)
		row.bind(3, nextWaypoint);
	else
		row.bind_null(3);

	row.bind(4, int(act));

	return true;
}

bool waypointClass::readFromRow(sqliteRowReference row, sequence id)
{
	row.assertWidth(5);

	_sequenceNum = id;
	
	if (!location.readFromRow(row.slice(0, 2)))
		return false;

	if (row.isnull(2)) {
		index = -1;
	} else {
		int64_t value = row.int64_field(2);
		if (value < 0 || value > 0x7FFF) {
			return false;
		}
		index = value;
	}

	if (row.isnull(3)) {
		nextWaypoint = -1;
	} else {
		nextWaypoint = row.int64_field(3);
	}

	act = static_cast<action>(row.int64_field(4));

	return true;
}

hackerboatStateStorage &waypointClass::storage() {
	static hackerboatStateStorage *waypointStorage;

	if (!waypointStorage) {
		waypointStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(WP_DB_FILE),
							     "WAYPOINT",
							     { { "latitude",   "REAL" },
							       { "longitude",  "REAL" },
							       { "index",      "INTEGER" },
							       { "next",       "INTEGER" },
							       { "action",     "INTEGER" } });
		waypointStorage->createTable();
	}

	return *waypointStorage;
}
