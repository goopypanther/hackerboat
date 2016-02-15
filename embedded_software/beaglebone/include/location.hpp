/******************************************************************************
 * Hackerboat Beaglebone location module
 * locations.hpp
 * This module stores locations and functions for manipulating them
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef LOCATION_H
#define LOCATION_H
 
#include <jansson.h>
#include <stdlib.h>
#include "config.h"
#include <math.h>

/**
 * @class locationClass
 *
 * @brief Class for storing a location 
 *
 */

class locationClass {
	public:
		enum courseType {
			GreatCircle,
			RhumbLine
		};
		locationClass (void) {};
		locationClass (double lat, double lon);		/**< Create a location object at the given latitude & longitude */
		bool isValid (void);						/**< Check for validity */
		bool parse (json_t *input);					/**< Populate the object from the given json object */
		json_t *pack (void);						/**< Pack the contents of the object into a json object and return a pointer to that object*/
		double bearing (locationClass dest, courseType type = GreatCircle);		/**< Get the bearing from the current location to the target */
		double distance (locationClass dest, courseType type = GreatCircle);

		double _lat = NAN;							/**< Latitude in degrees north of the equator. Values from -90.0 to 90.0, inclusive. */
		double _lon = NAN;							/**< Longitude in degrees east of the prime meridian. Values from -180.0 to 180.0, inclusive. */		
	
		static double inline deg2rad (double deg);
		static double inline rad2deg (double rad);
	
	protected:
		char *getFormatString(void) {return _format;};		/**< Get format string for the object */
		
	private:
		static const char *_format = "{s:f,s:f}";	
};

#endif /* LOCATION_H */