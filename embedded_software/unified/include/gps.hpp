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
#include <chrono>
#include <string>
#include "hackerboatRoot.hpp"
#include "sqliteStorage.hpp"
#include "location.hpp"

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
		gpsFixClass ();
		gpsFixClass (std::string sentence);			/**< Create a GPS fix from an incoming sentence string */
		
		std::chrono::time_point<std::chrono::system_clock>	uTime;					/**< GPS time of last fix */
		locationClass	fix;
		double		gpsHeading;				/**< True heading, according to GPS */
		double		gpsSpeed;				/**< Speed over the ground */
		bool 		fixValid;				/**< Checks whether this fix is valid or not */				
		bool parseGPDdPacket (json_t *packet);
		
	private:
		hackerboatStateStorage *gpsStorage = NULL;
};

#endif
