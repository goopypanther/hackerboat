/******************************************************************************
 * Hackerboat Beaglebone navigation module
 * waypoint.cpp
 * This module stores and processes navigation data
 * see the Hackerboat documentation for more details
 * 
 * Version 0.0: -
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include "config.h"
#include "stateStructTypes.hpp"
#include "location.hpp"
#include "navigation.hpp"
#include "sqliteStorage.hpp"

waypointClass::waypointClass()
	: act(CONTINUE)
{
}

waypointClass::waypointClass (locationClass loc, actionEnum action)
	: location(loc), act(action)
{
}

bool waypointClass::parse(json_t *input, bool seq)
{
#warning Pierce - implement me
	return false;
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
				    json_string(string(act)));

	return repr;
}

bool waypointClass::isValid(void) const
{
	if (!location.isValid())
		return false;

	if (index < 0 || nextWaypoint < 0)
		return false;

#warning Pierce - implement me

	return true;
}

const char *string(enum waypointClass::actionEnum act)
{
	switch(act) {
	case waypointClass::CONTINUE:
		return "CONTINUE";
	case waypointClass::STOP:
		return "STOP";
	case waypointClass::HOME:
		return "HOME";
	}
	return NULL;
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

	act = static_cast<actionEnum>(row.int64_field(4));

	return true;
}

static const char *sqlColumns[] = {
	"lat",
	"lon",
	"index",
	"next",
	"action"
};

static const char *columnTypes[] = {
	"REAL", "REAL",
	"INTEGER", "INTEGER",
	"INTEGER"
};

hackerboatStateStorage &waypointClass::storage() {
	static hackerboatStateStorage *waypointStorage;

	if (!waypointStorage) {
		waypointStorage = new hackerboatStateStorage(0, "WAYPOINT", 
							     std::vector<const char *>(sqlColumns, sqlColumns+4));
		waypointStorage->createTable(columnTypes);
	}

	return *waypointStorage;
}
