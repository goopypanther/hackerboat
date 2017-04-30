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

#include "rapidjson/rapidjson.h"
#include <stdlib.h>
#include <chrono>
#include <string>
#include "hackerboatRoot.hpp"
#include "location.hpp"
#include "enumtable.hpp"

using namespace rapidjson;

enum class NMEAModeEnum : int {
	NONE 	= 0,	/**< No data */
	NOFIX 	= 1,	/**< No valid fix */
	FIX2D	= 2,	/**< 2D fix only */
	FIX3D	= 3		/**< 3D fix */
};

/**
 * @class GPSFix 
 * 
 * @brief A GPS fix of some type
 *
 * The actual text of each incoming sentence is stored for logging purposes as well. 
 *
 */
class GPSFix : public HackerboatState {
	public:
		static const EnumNameTable<NMEAModeEnum> NMEAModeNames;
	
		GPSFix ();
		GPSFix (Value& packet);					/**< Create a GPS fix from an incoming gpsd TPV */
		GPSFix (const GPSFix& g) {this->copy(g);};
		bool parseGpsdPacket (Value& packet);	/**< Parse an incoming TSV into the current object. */
		bool parse (Value& input);
		Value pack () const;
		bool isValid () const;
		void copy(const GPSFix* newfix);
		void copy(const GPSFix& newfix);
		
		sysclock		gpsTime;	/**< GPS time of fix */
		
		NMEAModeEnum	mode = NMEAModeEnum::NONE;		/**< Mode of the fix */
		std::string		device = "";	/**< Name of the device */
		Location		fix;			/**< Location of the current fix */
		double			track = 0;		/**< Course over ground, in degrees from north */
		double			speed = 0;		/**< Speed over the ground in m/s */
		double 			alt = 0;		/**< Altitude, meters */
		double			climb = 0;		/**< Climb or sink rate, m/s */
		double			epx = 0;		/**< Longitude error, 95% confidence, meters */			
		double			epy = 0;		/**< Latitude error, 95% confidence, meters */
		double 			epd = 0;		/**< Track error, 95% confidence, degrees */	
		double			eps = 0;		/**< Speed error, 95% confidence, m/s */
		double			ept = 0;		/**< Timestamp error, 95% confidence, seconds */
		double 			epv = 0;		/**< Vertical error, 95% confidence, meters */
		double			epc = 0;		/**< Climb error, 95% confidence, m/s */

		bool 			fixValid = false;	/**< Checks whether this fix is valid or not */				
		
	private:
		bool coreParse (Value& input);	/**< This is the pieces of the parsing task shared between parse() and parseGpsdPacket() */
};

#endif
