/******************************************************************************
 * Hackerboat Beaglebone GPS module
 * gps.hpp
 * This module houses the GPS module
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef GPS_H
#define GPS_H

#include <jansson.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <inttypes.h>
#include <time.h>
#include "config.h"
#include "location.hpp"
#include "stateStructTypes.hpp"

#include <string>
using namespace string;

/**
 * @class gpsFixClass 
 * 
 * @brief A GPS fix of some type
 *
 * The actual text of each incoming sentence is stored for logging purposes as well. 
 *
 */
 
class gpsFixClass : public hackerboatStateClassStorable {
	public:
		gpsFixClass (const string file, string sentence);
		gpsFixClass (string sentence);			/**< Create a GPS fix from an incoming sentence string */
		
		bool readSentence (string sentence);	/**< Populate class from incoming sentence string */
		
		timespec	uTime;					/**< Beaglebone time of last fix */
		double		latitude;				/**< Latitude of last fix */
		double		longitude;				/**< Longitude of last fix */
		double		gpsHeading;				/**< True heading, according to GPS */
		double		gpsSpeed;				/**< Speed over the ground */
		string		GGA;					/**< GGA sentence from GPS */
		string		GSA;					/**< GSA sentence from GPS */
		string		GSV;					/**< GSV sentence from GPS */
		string		VTG;					/**< VTG sentence from GPS */
		string		RMC;					/**< RMC sentence from GPS */
	
	private:
		static const string _format = "{s:o,s:f,s:f,s:f,s:f,s:s,s:s,s:s,s:s,s:s}";
		static const double minHeading 		= 0.0;
		static const double maxHeading 		= 360.0;
		static const double minLatitude 	= -90.0;
		static const double maxLatitutde 	= 90.0;
		static const double minLongitude 	= -180.0;
		static const double maxLongitude 	= 180.0;
		static const double minHeading 		= 0.0;
		static const double maxHeading 		= 360.0;
		static const double minSpeed 		= 0.0;
};

#endif