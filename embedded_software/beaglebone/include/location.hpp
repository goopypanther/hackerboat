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
#include "config.h"
#include <math.h>

class sqliteParameterSlice;
class sqliteRowReference;

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
		locationClass (void)
		  : _lat(NAN), _lon(NAN)
		{ };
		locationClass (double lat, double lon)					/**< Create a location object at the given latitude & longitude */
		  : _lat(lat), _lon(lon)
		{ };
		bool isValid (void) const;						/**< Check for validity */
		bool parse (json_t *input);						/**< Populate the object from the given json object */
		json_t *pack (void) const;						/**< Pack the contents of the object into a json object and return a pointer to that object*/
		double bearing (const locationClass& dest, courseType type = GreatCircle) const;	/**< Get the bearing from the current location to the target */
		double distance (const locationClass& dest, courseType type = GreatCircle) const;

		double _lat;								/**< Latitude in degrees north of the equator. Values from -90.0 to 90.0, inclusive. */
		double _lon;								/**< Longitude in degrees east of the prime meridian. Values from -180.0 to 180.0, inclusive. */		
	
		static double inline deg2rad (double deg) { return deg * ( M_PI / 180.0 ); }
		static double inline rad2deg (double rad) { return rad * ( 180.0 / M_PI ); }

		bool fillRow(sqliteParameterSlice) const;
		bool readFromRow(sqliteRowReference);

	protected:
		static const char * const _format;
};

#endif /* LOCATION_H */
