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
#include <sqlite3.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <array>
#include <string>
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"

json_t *hackerboatStateClass::packTimeSpec (timespec t) {
	return json_pack("{s:i,s:i}", "tv_sec", t.tv_sec, "tv_nsec", t.tv_nsec);
}

int hackerboatStateClass::parseTimeSpec (json_t *input, timespec *t) {
	return json_unpack(input, "{s:i,s:i}", "tv_sec", &(t->tv_sec), "tv_nsec", &(t->tv_nsec));
}

json_t *json(std::string const v)
{
	return json_stringn(v.data(), v.length());
}

json_t *json(bool v) {
	return json_boolean(v);
}

json_t *orientationClass::pack (bool seq) {
	json_t *output = json_pack(this->_format.c_str(),
								"roll", roll,
								"pitch", pitch,
								"yaw", yaw);
	return output;
}

bool orientationClass::parse (json_t *input, bool seq = true) {
	if (json_unpack(input, this->_format.c_str(),					
					"roll", roll,
					"pitch", pitch,
					"yaw", yaw)) {
		return false;
	}
	return this->isValid();
}

bool orientationClass::isValid (void) {
	return this->normalize();
}

bool orientationClass::normalize (void) {
	bool result = true;
	if (isfinite(roll)) {roll = fmod(roll, maxVal);} else {result = false;}
	if (isfinite(pitch)) {pitch = fmod(pitch, maxVal);} else {result = false;}
	if (isfinite(yaw)) {yaw = fmod(yaw, maxVal);} else {result = false;}
	return result;
}

