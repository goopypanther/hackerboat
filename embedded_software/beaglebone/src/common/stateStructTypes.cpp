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
#include <string>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "sqliteStorage.hpp"

json_t *hackerboatStateClass::packTimeSpec (timespec t) {
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "tv_sec", json_integer(t.tv_sec));
	packResult += json_object_set_new(output, "tv_nsec", json_integer(t.tv_nsec));
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
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
	json_t *output = json_object();
	if (this->isValid()) {
		json_object_set_new(output, "roll", json_real(roll));
		json_object_set_new(output, "pitch", json_real(pitch));
		json_object_set_new(output, "yaw", json_real(heading));
	} else {
		json_object_set_new(output, "roll", json_real(0));
		json_object_set_new(output, "pitch", json_real(0));
		json_object_set_new(output, "yaw", json_real(0));
	}
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

