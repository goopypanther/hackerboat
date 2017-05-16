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

extern "C" {
	#include <jansson.h>
}
#include <stdlib.h>
#include <chrono>
#include <string>
#include "hackerboatRoot.hpp"
#include "sqliteStorage.hpp"
#include "location.hpp"
#include "enumtable.hpp"

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
class GPSFix : public HackerboatStateStorable {
	public:
		static const EnumNameTable<NMEAModeEnum> NMEAModeNames;
	
		GPSFix ();
		GPSFix (json_t *packet);				/**< Create a GPS fix from an incoming gpsd TPV */
		bool parseGpsdPacket (json_t *packet);	/**< Parse an incoming TSV into the current object. */
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid () const;
		HackerboatStateStorage& storage();
		bool fillRow(SQLiteParameterSlice) const USE_RESULT;
		bool readFromRow(SQLiteRowReference, sequence) USE_RESULT;
		void copy(GPSFix *newfix);
		void copy(GPSFix &newfix);
		
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
		bool coreParse (json_t* input);	/**< This is the pieces of the parsing task shared between parse() and parseGpsdPacket() */
		HackerboatStateStorage *gpsStorage = NULL;
};

#endif
