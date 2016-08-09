/******************************************************************************
 * Hackerboat Beaglebone orientation module
 * orientation.hpp
 * This module stores orientations and functions for manipulating them
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef ORIENTATION_H
#define ORIENTATION_H
 
#include <jansson.h>
#include "config.h"
#include <math.h>
#include <string>
#include "hackerboatRoot.hpp"

/**
 * @class orientationClass
 *
 * @brief An orientation calculated from data received from the IMU
 *
 */

class orientationClass : public hackerboatStateClass {
	public:
		orientationClass() {};
		orientationClass(double r, double p, double y)
		  : pitch(p), roll(r), heading(y)
		{};
		bool normalize (void);
		double roll 	= NAN;
		double pitch 	= NAN;
		double heading 	= NAN;

		bool parse (json_t *) USE_RESULT;
		json_t *pack (void) const;
		bool isValid (void) const;

	private:
		static const double constexpr	maxVal = 180.0;
		static const double constexpr	minVal = -180.0;
};

#endif