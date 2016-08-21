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
#include "hal/config.h"
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
		orientationClass() = default;
		orientationClass(double r, double p, double y) :
			pitch(p), roll(r), heading(y) {};
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid ();
		bool normalize (void);		/**< Normalize the roll/pitch/heading */
		double roll 	= NAN;
		double pitch 	= NAN;
		double heading 	= NAN;

	private:
		static const double constexpr	maxVal = 180.0;
		static const double constexpr	minVal = -180.0;
};

#endif
