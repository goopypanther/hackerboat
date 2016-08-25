/******************************************************************************
 * Hackerboat Beaglebone location module
 * location.cpp
 * This module stores locations and functions for manipulating them
 * Bearing and distance formulas from http://www.movable-type.co.uk/scripts/latlong.html
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 * Version 0.2: Compiles
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <cmath>
#include "location.hpp"
#include "json_utilities.hpp"
#include "twovector.hpp"

/* The radius of Earth, in meters. (We're using a spherical-Earth approximation.) */
#define R 6731000

/* Reading and writing from JSON and the database. */

bool Location::isValid(void) const {
	return ((lat <= 90.0) && (lat >= -90) && 
			(lon <= 180.0) && (lon >= -180) &&
			(std::isnormal(lat)) && (std::isnormal(lon)));
}

bool Location::parse(json_t* input) {
	::parse(json_object_get(input, "lat"), &lat);
	::parse(json_object_get(input, "lon"), &lon);
	return this->isValid();
}

json_t* Location::pack(void) const {
	json_t *output = json_object();
	int packResult = 0;
	if (this->isValid()) {
		packResult += json_object_set_new(output, "lat", json_real(lat));
		packResult += json_object_set_new(output, "lon", json_real(lon));
	} else {
		packResult += json_object_set_new(output, "lat", json_real(0));
		packResult += json_object_set_new(output, "lon", json_real(0));
	}
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

/* Computation methods. */

double Location::bearing (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double deltaLon = TwoVector::deg2rad(dest.lon - lon);
	double radLat1 = TwoVector::deg2rad(lat);
	double radLat2 = TwoVector::deg2rad(dest.lat);
	switch (type) {
		case GreatCircle:
			return TwoVector::rad2deg(atan2((sin(deltaLon)*cos(radLat2)),
				     ((cos(radLat1)*sin(radLat2)) -
				      (sin(radLat1)*cos(radLat2)*cos(deltaLon)))));
		case RhumbLine:
		  {
			double deltaPsi = log(tan(M_PI_4+(radLat2/2))/
					      tan(M_PI_4+(radLat1/2)));
			return TwoVector::rad2deg(atan2(deltaLon, deltaPsi));
		  }
		default:
			return NAN;
	}
}

double Location::distance (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double deltaLon = TwoVector::deg2rad(dest.lon - lon);
	double deltaLat = TwoVector::deg2rad(dest.lat - lat);
	double radLat1 = TwoVector::deg2rad(lat);
	double radLat2 = TwoVector::deg2rad(dest.lat);
	switch (type) {
		case GreatCircle:
		  {
			double a = pow(sin(deltaLat/2), 2) + (cos(radLat1) * cos(radLat2) * pow(sin(deltaLon/2), 2));
			double c = 2 * atan2(sqrt(a), sqrt(1-a));
			return (R * c);
		  }
		case RhumbLine:
		  {
			double deltaPsi = log(tan(M_PI_4+(radLat2/2))/
					      tan(M_PI_4+(radLat1/2)));
			double q;
			if (abs(deltaLat) < 0.0001) {
				q = cos(radLat1);
			} else {
				q = deltaLat/deltaPsi;
			}
			return (R * sqrt((deltaLat * deltaLat) + (q * q * deltaLon * deltaLon)));
		  }
		default:
			return NAN;
	}
	
}

