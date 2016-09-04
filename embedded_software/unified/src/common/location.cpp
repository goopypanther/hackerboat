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

/* The radius of Earth, in meters. (We're using a spherical-Earth approximation.) */
#define R (6239837)
/* Minimum angle (close to roundoff error) */
#define MIN_ANG	0.0000001

/* Reading and writing from JSON and the database. */

bool Location::isValid(void) const {
	return ((lat <= 90.0) && (lat >= -90) && 
			(lon <= 180.0) && (lon >= -180) &&
			(std::isfinite(lat)) && (std::isfinite(lon)));
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
		case CourseTypeEnum::GreatCircle:
			return TwoVector::rad2deg(atan2((sin(deltaLon)*cos(radLat2)),
				     ((cos(radLat1)*sin(radLat2)) -
				      (sin(radLat1)*cos(radLat2)*cos(deltaLon)))));
			break;
		case CourseTypeEnum::RhumbLine:	{
			double deltaPsi = log(tan(M_PI_4+(radLat2/2))/
						  tan(M_PI_4+(radLat1/2)));
			return TwoVector::rad2deg(atan2(deltaLon, deltaPsi));
			break;
		}
		default:
			return NAN;
	}
}

double Location::distance (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double deltaLon = TwoVector::deg2rad(dest.lon - this->lon);
	double deltaLat = TwoVector::deg2rad(dest.lat - this->lat);
	double radLat1 = TwoVector::deg2rad(this->lat);
	double radLat2 = TwoVector::deg2rad(dest.lat);
	switch (type) {
		case CourseTypeEnum::GreatCircle:{
			double a = pow(sin(deltaLat/2), 2) + (cos(radLat1) * cos(radLat2) * pow(sin(deltaLon/2), 2));
			double c = 2 * atan2(sqrt(a), sqrt(1-a));
			return (R * c);	// convert from radians to distance, given a spherical Earth
			break;
		}
		case CourseTypeEnum::RhumbLine: {
			double deltaPsi = log(tan(M_PI_4+(radLat2/2))/
						  tan(M_PI_4+(radLat1/2)));
			double q;
			if (abs(deltaLat) < MIN_ANG) {
				q = cos(radLat1);
			} else {
				q = deltaLat/deltaPsi;
			}
			return (R * sqrt((deltaLat * deltaLat) + (q * q * deltaLon * deltaLon)));
			break;
		}
		default:
			return NAN;
	}
	
}

TwoVector Location::target (const Location& dest, CourseTypeEnum type) const {
	if ((!this->isValid()) || (!dest.isValid())) return TwoVector {NAN,NAN};
	TwoVector direction {1, 0};							// Create a unit vector pointed due north
	direction.rotateDeg(this->bearing(dest, type));		// Rotate to the desired course
	direction *= this->distance(dest, type);			// multiply by the correct 					
	return direction;
}

Location Location::project (TwoVector& projection, CourseTypeEnum type) {
	double d = (projection.mag())/R;				// distance traveled in radians, given a spherical Earth
	double tc = (projection.angleRad());			// course in radians
	double radLat1 = TwoVector::deg2rad(this->lat);	// latitude, in radians
	double q;								
	Location result;
	switch (type) {
		case CourseTypeEnum::GreatCircle: {
			double radLat = asin(sin(radLat1)*cos(d)+cos(radLat1)*sin(d)*cos(tc));
			double dlon = atan2((sin(tc)*sin(d)*cos(radLat1)),(cos(d)-(sin(radLat1)*sin(radLat))));
			result.lat = TwoVector::rad2deg(radLat);
			result.lon = TwoVector::rad2deg(fmod((radLat1 + dlon + M_PI),(2 * M_PI)));
			break;
		}
		case CourseTypeEnum::RhumbLine: {
			double radLat = radLat1 + (d * cos(tc));
			if (abs(radLat) > (M_PI_2)) return result;
			if (abs(radLat - radLat1) < MIN_ANG) {
				q = cos(radLat);
			} else {
				double dphi = log10(tan((radLat/2) + M_PI_4))/(tan((radLat1/2) + M_PI_4));
				q = (radLat - radLat1)/dphi;
			}
			double dlon = -d * sin(tc)/q;
			result.lat = TwoVector::rad2deg(radLat);
			result.lon = TwoVector::rad2deg(fmod((radLat1 + dlon + M_PI),(2 * M_PI)));
			break;
		}
		default:
			break;
	}
	return result;
} 


