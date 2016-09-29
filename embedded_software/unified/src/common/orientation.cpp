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
#include <chrono>
#include <ctime>
#include "orientation.hpp"
#include <GeographicLib/MagneticModel.hpp>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/Constants.hpp>

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

using namespace GeographicLib;

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
	return (std::isfinite(roll) && std::isfinite(pitch) && std::isfinite(heading));
}

bool Orientation::normalize () {
	pitch 	+= 180.0;
	roll 	+= 180.0;
	pitch 	= normAxis(pitch, 360.0, 0.0);
	roll 	= normAxis(roll, 360.0, 0.0);
	heading = normAxis(heading, 360.0, 0.0);
	pitch 	-= 180.0;
	roll 	-= 180.0;
	
	return this->isValid();
}

double Orientation::normAxis (double val, const double max, const double min) const {
	double span = (max - min);
	while (val < min) val += span;
	while (val > max) val -= span;
	return val;
}

double Orientation::headingError (double target) {
	if (!isValid()) return NAN;
	return normAxis((target - heading), 180.0, -180.0);
}

Orientation Orientation::makeTrue () {
	if (!this->isMagnetic()) return *this;
	Orientation output = *this;
	output.magnetic = false;
	output.heading += declination;
	output.normalize();
	return output;
}

Orientation Orientation::makeMag () {
	if (this->isMagnetic()) return *this;
	Orientation output = *this;
	output.magnetic = true;
	output.heading -= declination;
	output.normalize();
	return output;
}

bool Orientation::updateDeclination (Location loc) {
	if (!loc.isValid()) return false;
	// time information for the mag model
	sysclock thisTime = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(thisTime);
	tm utc_tm = *gmtime(&tt);
	MagneticModel mag("emm2015");
	// intermediate values
	double Bx, By, Bz, H;
	// output values
	double strength, inclination;
	
	// Grab the magnetic components
	mag(utc_tm.tm_year + 1900, loc.lat, loc.lon, 0, Bx, By, Bz);
	
	// convert intermediate representation to the output
	MagneticModel::FieldComponents(Bx, By, Bz, H, strength, declination, inclination);
	
	return true;
}


double Orientation::declination = 0;