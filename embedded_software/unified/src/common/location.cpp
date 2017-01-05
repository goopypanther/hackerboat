/******************************************************************************
 * Hackerboat Beaglebone location module
 * location.cpp
 * This module stores locations and functions for manipulating them
 * Navigation formulas from Ed Williams' Aviation Formulary
 * http://williams.best.vwh.net/avform.htm via
 * http://www.movable-type.co.uk/scripts/latlong.html
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 * Version 0.2: Compiles
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include "location.hpp"
#include "json_utilities.hpp"
#include "twovector.hpp"
#include "easylogging++.h"

using namespace GeographicLib;

/* Minimum angle (close to roundoff error) */
#define MIN_ANG	0.0000001

/* Reading and writing from JSON and the database. */

bool Location::isValid(void) const {
	return ((lat <= 90.0) && (lat >= -90) && 
			(lon <= 180.0) && (lon >= -180) &&
			(std::isfinite(lat)) && (std::isfinite(lon)));
}

bool Location::parse(json_t* input) {
	LOG(DEBUG) << "Parsing a location object from " << input;
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
		LOG(ERROR) << "Location packing failed " << output;
		json_decref(output);
		return NULL;
	} else {
		LOG(DEBUG) << "Packed location object: " << output;
		return output;
	}
}

/* Computation methods. */

double Location::bearing (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double azi1, azi2, dist;
	switch (type) {
		case CourseTypeEnum::GreatCircle: {
			geod->Inverse(this->lat, this->lon, dest.lat, dest.lon, azi1, azi2);
			VLOG(3) << "Great circle bearing to " << dest << " is " << to_string(azi1);
			return azi1;
			break;
		}
		case CourseTypeEnum::RhumbLine:	{
			rhumb->Inverse(this->lat, this->lon, dest.lat, dest.lon, dist, azi1);
			VLOG(3) << "Rhumb line bearing to " << dest << " is " << to_string(azi1);
			return azi1;
			break;
		}
		default:
			return NAN;
	}
	return NAN;
}

double Location::distance (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double azi1, dist;
	switch (type) {
		case CourseTypeEnum::GreatCircle: {
			geod->Inverse(this->lat, this->lon, dest.lat, dest.lon, dist);
			VLOG(3) << "Great circle distance to " << dest << " is " << to_string(dist) << "m";
			return dist;
			break;
		}
		case CourseTypeEnum::RhumbLine: {
			rhumb->Inverse(this->lat, this->lon, dest.lat, dest.lon, dist, azi1);
			VLOG(3) << "Rhumb line distance to " << dest << " is " << to_string(dist) << "m";
			return dist;
			break;
		}
		default:
			return NAN;
	}
	
}

TwoVector Location::target (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) {
		LOG(DEBUG) << "Attempting to get vector from invalid position " << *this << " or to invalid location " << dest;
		return TwoVector {NAN,NAN};
	}
	TwoVector direction {1, 0};							// Create a unit vector pointed due north
	direction.rotateDeg(this->bearing(dest, type));		// Rotate to the desired course
	direction *= this->distance(dest, type);			// multiply by the correct 					
	return direction;
}

Location Location::project (TwoVector& projection, CourseTypeEnum type) {					
	Location result;
	switch (type) {
		case CourseTypeEnum::GreatCircle: {
			geod->Direct(this->lat, this->lon, projection.angleDeg(), projection.mag(), result.lat, result.lon);
			VLOG(3) << "Great circle projection along " << projection << " is " << result;
			return result;
			break;
		}
		case CourseTypeEnum::RhumbLine: {
			rhumb->Direct(this->lat, this->lon, projection.angleDeg(), projection.mag(), result.lat, result.lon);
			VLOG(3) << "Rhumb line projection along " << projection << " is " << result;
			break;
		}
		default:
			break;
	}
	return result;
} 

Geodesic *Location::geod = new Geodesic(Constants::WGS84_a(), Constants::WGS84_f());
Rhumb *Location::rhumb = new Rhumb(Constants::WGS84_a(), Constants::WGS84_f());


