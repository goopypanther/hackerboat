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
#include <math.h>
#include "location.hpp"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"

/* The radius of Earth, in meters. (We're using a spherical-Earth approximation.) */
#define R 6731000

/* Reading and writing from JSON and the database. */

bool locationClass::isValid(void) const {
	return ((_lat <= 90.0) && (_lat >= -90) && 
			(_lon <= 180.0) && (_lon >= -180) &&
			(isnormal(_lat)) && (isnormal(_lon)));
}

bool locationClass::parse(json_t* input) {
	::parse(json_object_get(input, "latitude"), &_lat);
	::parse(json_object_get(input, "longitude"), &_lon);
	return this->isValid();
}

json_t* locationClass::pack(void) const {
	json_t *output = json_object();
	int packResult = 0;
	if (this->isValid()) {
		packResult += json_object_set_new(output, "latitude", json_real(_lat));
		packResult += json_object_set_new(output, "longitude", json_real(_lon));
	} else {
		packResult += json_object_set_new(output, "latitude", json_real(0));
		packResult += json_object_set_new(output, "longitude", json_real(0));
	}
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	} else return output;
}

bool locationClass::fillRow(sqliteParameterSlice row) const {
	row.assertWidth(2);
	row.bind(0, _lat);
	row.bind(1, _lon);
	return true;
}

bool locationClass::readFromRow(sqliteRowReference row) {
	row.assertWidth(2);
	_lat = row.double_field(0);
	_lon = row.double_field(1);
	return isValid();
}

/* Computation methods. */

double locationClass::bearing (const locationClass& dest, courseType type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double deltaLon = deg2rad(dest._lon - _lon);
	double radLat1 = deg2rad(_lat);
	double radLat2 = deg2rad(dest._lat);
	switch (type) {
		case GreatCircle:
			return atan2((sin(deltaLon)*cos(radLat2)),
				     ((cos(radLat1)*sin(radLat2)) -
				      (sin(radLat1)*cos(radLat2)*cos(deltaLon))));
		case RhumbLine:
		  {
			double deltaPsi = log(tan(M_PI_4+(radLat2/2))/
					      tan(M_PI_4+(radLat1/2)));
			return atan2(deltaLon, deltaPsi);
		  }
		default:
			return NAN;
	}
}

double locationClass::distance (const locationClass& dest, courseType type) const {
	if ((!this->isValid()) || (!dest.isValid())) return NAN;
	double deltaLon = deg2rad(dest._lon - _lon);
	double deltaLat = deg2rad(dest._lat - _lat);
	double radLat1 = deg2rad(_lat);
	double radLat2 = deg2rad(dest._lat);
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

