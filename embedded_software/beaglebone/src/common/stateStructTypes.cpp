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
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <array>
#include <string>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"

json_t *hackerboatStateClass::packTimeSpec (timespec t) {
	return json_pack("{s:i,s:i}", "tv_sec", t.tv_sec, "tv_nsec", t.tv_nsec);
}

int hackerboatStateClass::parseTimeSpec (json_t *input, timespec *t) {
	return json_unpack(input, "{s:i,s:i}", "tv_sec", &(t->tv_sec), "tv_nsec", &(t->tv_nsec));
}

bool hackerboatStateClassStorable::fillRow(sqliteParameterSlice row) const {
	row.assertWidth(1);
	json_t *representation = this->pack(false);
	if (!representation)
		return false;
	row.bind_json_new(0, representation);
	return true;
}

bool hackerboatStateClassStorable::readFromRow(sqliteRowReference row, sequence seq) {
	row.assertWidth(1);
	_sequenceNum = seq;
	json_t *representation = row.json_field(0);
	if (!representation)
		return false;
	bool success = this->parse(representation, false);
	json_decref(representation);
	return success;
}

json_t *orientationClass::pack () const {
	json_t *output = json_pack("{s:f,s:f,s:f}",
				   "roll", roll,
				   "pitch", pitch,
				   "yaw", heading);
	return output;
}

bool orientationClass::parse (json_t *input) {
	if (json_unpack(input, "{s:F,s:F,s:F}",
			"roll", &roll,
			"pitch", &pitch,
			"yaw", &heading)) {
		return false;
	}
	return this->normalize();
}

bool orientationClass::isValid (void) const {
	return isfinite(roll) && isfinite(pitch) && isfinite(heading);
}

bool orientationClass::normalize (void) {
	bool result = true;
	if (isfinite(roll)) {roll = fmod(roll, maxVal);} else {result = false;}
	if (isfinite(pitch)) {pitch = fmod(pitch, maxVal);} else {result = false;}
	if (isfinite(heading)) {heading = fmod(heading, maxVal);} else {result = false;}
	return result;
}

