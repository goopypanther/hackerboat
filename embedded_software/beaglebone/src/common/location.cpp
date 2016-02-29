/******************************************************************************
 * Hackerboat Beaglebone location module
 * location.cpp
 * This module stores locations and functions for manipulating them
 * Bearing and distance formulas from http://www.movable-type.co.uk/scripts/latlong.html
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include "location.hpp"

#define PI 3.14159265
#define R 6731000

static double inline locationClass::deg2rad (double deg) {
	return ((deg * PI)/180);
}

static double inline locationClass::rad2deg (double rad) {
	return ((rad * 180)/PI);
}

locationClass::locationClass (double lat, double lon) {
	_lat = lat;
	_lon = lon;
}

bool inline locationClass::isValid(void) const {
	return ((_lat <= 90.0) && (_lat >= -90) && 
			(_lon <= 180.0) && (_lon >= -180) &&
			(isnormal(_lat)) && (isnormal(_lon)));
}

bool locationClass::parse(json_t* input) {
	if (json_unpack(input, this->_format, "latitude", &_lat, "longitude", &_lon)) {
		return false;
	}
	return this->isValid();
}

json_t* locationClass::pack(void) {
	return json_pack(this->_format, "latitude", _lat, "longitude", _lon);
}

double locationClass::bearing (locationClass dest, courseType type) {
	if ((!this->isValid()) || (!dest->isValid())) return NAN;
	double deltaLon = deg2rad(dest->_lon - this->_lon);
	double radLat1 = deg2rad(this->_lat);
	double radLat2 = deg2rad(dest->_lat);
	switch (type) {
		case GreatCircle:
			return atan2((sin(deltaLon)*cos(radLat2)),
						((cos(radLat1)*sin(radLat2)) -
						(sin(radLat1)*cos(radLat2)*cos(deltaLon))));
		case RhumbLine:
			double deltaPsi = log(tan((PI/4)+(radLat2/2))/
							tan((PI/4)+(radLat1/2)));
			return atan2(deltaLon, deltaPsi);
		default:
			return NAN;
	}
}

double locationClass::distance (locationClass dest, courseType type) {
	if ((!this->isValid()) || (!dest->isValid())) return NAN;
	double deltaLon = deg2rad(dest->_lon - this->_lon);
	double deltaLat = deg2rad(dest->_lat - this->_lat);
	double radLat1 = deg2rad(this->_lat);
	double radLat2 = deg2rad(dest->_lat);
	switch (type) {
		case GreatCircle:
			double a = pow(sin(deltaLat/2), 2) + (cos(radLat1) * cos(radLat2) * pow(sin(deltaLon/2), 2));
			double c = 2 * atan2(sqrt(a), sqrt(1-a));
			return (R * c);
		case RhumbLine:
			double deltaPsi = log(tan((PI/4)+(radLat2/2))/
							tan((PI/4)+(radLat1/2)));
			double q;
			if (abs(deltaLat) < 0.0001) {
				q = cos(radLat1);
			} else {
				q = deltaLat/deltaPsi;
			}
			return (R * sqrt((deltaLat * deltaLat) + (q * q * deltaLon * deltaLon)));
		default:
			return NAN;
	}
	