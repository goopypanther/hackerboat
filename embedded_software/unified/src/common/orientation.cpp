/******************************************************************************
 * Hackerboat Beaglebone orientation module
 * orientation.cpp
 * This module stores orientations and functions for manipulating them
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <cmath>
#include <cstdlib>
#include <string>
#include <jansson.h>
#include "orientation.hpp"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

bool Orientation::parse (json_t *input) {
	return(GET_VAR(pitch) && GET_VAR(heading) && GET_VAR(roll) && this->isValid());
}

json_t *Orientation::pack () const {
	json_t *output = json_object();
	int packResult = 0;
	
	packResult += json_object_set_new(output, "pitch", json_integer(pitch));
	packResult += json_object_set_new(output, "roll", json_integer(roll));
	packResult += json_object_set_new(output, "heading", json_integer(heading));
	
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool Orientation::isValid () {
	return (std::isnormal(roll) && std::isnormal(pitch) && std::isnormal(heading));
}

bool Orientation::normalize () {
	pitch = normAxis(pitch, maxPitch, minPitch);
	roll = normAxis(roll, maxRoll, minRoll);
	heading = normAxis(heading, maxHeading, minHeading);
	
	return this->isValid();
}

double Orientation::normAxis (double val, const double max, const double min) const {
	if (val > max) return fmod(val, max);
	while (val < min) val += (max - min);
	if (val > max) return fmod(val, max);
	return val;
}
