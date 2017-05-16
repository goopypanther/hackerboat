/******************************************************************************
 * Hackerboat Vector module
 * twovector.cpp
 * This module manipulates two dimensional vectors 
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <stdlib.h>
#include <cmath> 
#include "hackerboatRoot.hpp"
#include "twovector.hpp"
#include "easylogging++.h"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

bool TwoVector::parse(Value& d) {
	return (GetVar("_x", _x, d) & GetVar("_y", _y, d));
}

Value TwoVector::pack () const {
	Value d;
	int r = 0;
	r += PutVar("_x", _x, d);
	r += PutVar("_y", _y, d);
	
	return d;
}

TwoVector TwoVector::getVectorRad(double ang, double mag) {
	TwoVector result {1, 0};	// create a unit vector at angle 0
	result.rotateRad(ang);		// rotate that vector through the given ang
	result *= mag;				// multiply by the desired magnitude
	return result;
}

/** 
 * @brief This function implements a simple two-dimensional rotation matrix.
 * See https://en.wikipedia.org/wiki/Rotation_matrix for details.
 */
void TwoVector::rotateRad (double _rad) {
	double x, y;
	x = (_x * cos(_rad)) - (_y * sin(_rad));
	y = (_x * sin(_rad)) + (_y * cos(_rad));
	_x = x;
	_y = y;
}	

TwoVector TwoVector::unit() {
	TwoVector result;
	result = *this * (1/this->mag());
	return result;
}