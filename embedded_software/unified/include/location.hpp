/******************************************************************************
 * Hackerboat Beaglebone location module
 * locations.hpp
 * This module stores locations and functions for manipulating them
 * Navigation formulas from Ed Williams' Aviation Formulary
 * http://williams.best.vwh.net/avform.htm
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef LOCATION_H
#define LOCATION_H
 
#include <jansson.h>
#include "hal/config.h"
#include <math.h>
#include <string>
#include "hackerboatRoot.hpp"
#include "twovector.hpp"

/**
 * @class Location
 *
 * @brief Class for storing a location 
 *
 */

class Location : public HackerboatState {
	public:
		enum CourseTypeEnum {
			GreatCircle,
			RhumbLine
		};
		Location (void)
		  : lat(NAN), lon(NAN)
		{ };
		Location (double _lat, double _lon)			/**< Create a location object at the given latitude & longitude */
		  : lat(_lat), lon(_lon)
		{ };
		bool parse (json_t *input);
		json_t *pack () const USE_RESULT;
		bool isValid (void) const;						/**< Check for validity */
		double bearing (const Location& dest, CourseTypeEnum type = GreatCircle) const;		/**< Get the bearing from the current location to the target */
		double distance (const Location& dest, CourseTypeEnum type = GreatCircle) const;	/**< Get the distance from the current location to the target */
		TwoVector target (const Location& dest, CourseTypeEnum type = GreatCircle) const;	/**< Get the course and distance to destination as a TwoVector object */
		Location projectm (const TwoVector& projection);									/**< Get the Location at the given meter-valued TwoVector from the current location */
		
		double lat;								/**< Latitude in degrees north of the equator. Values from -90.0 to 90.0, inclusive. */
		double lon;								/**< Longitude in degrees east of the prime meridian. Values from -180.0 to 180.0, inclusive. */		

};

#endif /* LOCATION_H */
