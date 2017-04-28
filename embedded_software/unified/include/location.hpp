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
 
#include "rapidjson/rapidjson.h"
#include "hal/config.h"
#include <math.h>
#include <string>
#include "hackerboatRoot.hpp"
#include "twovector.hpp"
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Rhumb.hpp>
#include <GeographicLib/Constants.hpp>

using namespace GeographicLib;
using namespace rapidjson;

enum class CourseTypeEnum {
	GreatCircle,
	RhumbLine,
	Approximate
};

/**
 * @class Location
 *
 * @brief Class for storing a location 
 *
 */

class Location : public HackerboatState {
	public:
		Location (void)
		  	: lat(NAN), lon(NAN) { };
		Location (double _lat, double _lon)			/**< Create a location object at the given latitude & longitude */
		  	: lat(_lat), lon(_lon) { };
		Location (const Location& l)
			: lat(l.lat), lon(l.lon) { };
		bool parse (Value& input);					/**< Populate this Location object from a properly formated json object */
		Value pack () const USE_RESULT;			/**< Pack a json object from this Location object. */
		bool isValid (void) const;					/**< Check for validity */
		double bearing (const Location& dest, CourseTypeEnum type = CourseTypeEnum::GreatCircle) const;		/**< Get the bearing from the current location to the target */
		double distance (const Location& dest, CourseTypeEnum type = CourseTypeEnum::GreatCircle) const;	/**< Get the distance from the current location to the target, in meters */
		TwoVector target (const Location& dest, CourseTypeEnum type = CourseTypeEnum::GreatCircle) const;	/**< Get the course and distance to destination as a TwoVector, in meters */
		Location project (TwoVector& projection, CourseTypeEnum type = CourseTypeEnum::GreatCircle);		/**< Get the Location at the given meter-valued TwoVector from the current location */
		Location& operator=(const Location& l) {
			this->lat = l.lat;
			this->lon = l.lon;
			return *this;
		}

		double 		lat;							/**< Latitude in degrees north of the equator. Values from -90.0 to 90.0, inclusive. */
		double 		lon;							/**< Longitude in degrees east of the prime meridian. Values from -180.0 to 180.0, inclusive. */		
	private:
		static Geodesic *geod;	
		static Rhumb 	*rhumb;
};

#endif /* LOCATION_H */
