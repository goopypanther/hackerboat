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
#include <cmath>
#include <string>
#include "hackerboatRoot.hpp"
#include "location.hpp"
#include <GeographicLib/MagneticModel.hpp>
#include <GeographicLib/Geocentric.hpp>

/**
 * @class Orientation
 *
 * @brief An orientation calculated from data received from the IMU
 *
 */

class Orientation : public HackerboatState {
	public:
		Orientation() = default;
		Orientation(double r, double p, double y) :
			pitch(p), roll(r), heading(y) {};
		bool parse (json_t *input);				/**< Parse an orientation object out of json object */
		json_t *pack () const;					/**< Create a json object of this orientation */
		bool isValid ();						/**< Check if this is a valid orientation object */
		bool normalize (void);					/**< Normalize the roll/pitch/heading values to +/-180 degrees (or 0-360 degrees in the case of heading) */
		double headingError (double target);	/**< Get the error angle between this Orientation and target heading, in degrees. */	
		Orientation makeTrue ();				/**< Return an Orientation object with the heading as a true (rather than magnetic) heading. Requires location to compute magnetic declination. */
		Orientation makeMag ();					/**< Return an Orientation object with the heading as a magnetic (rather than true) heading. Requires location to compute magnetic declination. */
		bool isMagnetic() {return magnetic;};	/**< Returns true if the heading is magnetic rather than true */ 
		bool updateDeclination(Location loc);
		double roll 	= NAN;			
		double pitch 	= NAN;
		double heading 	= NAN;

	private:
		double normAxis (double val, const double max, const double min) const;		/**< Normalize given axis */
		bool magnetic = true;
		static double declination;
		static const double constexpr	maxRoll = 180.0;
		static const double constexpr	minRoll = -180.0;
		static const double constexpr	maxPitch = 180.0;
		static const double constexpr	minPitch = -180.0;
		static const double constexpr	maxHeading = 360.0;
		static const double constexpr	minHeading = 0.0;
	
};

#endif
