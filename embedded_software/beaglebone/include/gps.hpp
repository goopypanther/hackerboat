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
#include <time.h>
#include <string>
#include "stateStructTypes.hpp"

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
		
		bool readSentence (std::string sentence);	/**< Populate class from incoming sentence string */
		
		timespec	uTime;					/**< Beaglebone time of last fix */
		timespec	gpsTime;				/**< GPS time of last fix */
		double		latitude;				/**< Latitude of last fix */
		double		longitude;				/**< Longitude of last fix */
		double		gpsHeading;				/**< True heading, according to GPS */
		double		gpsSpeed;				/**< Speed over the ground */
		bool 		fixValid;				/**< Checks whether this fix is valid or not */				
		std::string	GGA;					/**< GGA sentence from GPS */
		std::string	GSA;					/**< GSA sentence from GPS */
		std::string	GSV;					/**< GSV sentence from GPS */
		std::string	VTG;					/**< VTG sentence from GPS */
		std::string	RMC;					/**< RMC sentence from GPS */
	
		/* Concrete implementations of stateClassStorable */
		bool parse (json_t *, bool);
		json_t *pack (bool seq = false) const;
		bool isValid (void) const;

	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
		virtual bool fillRow(sqliteParameterSlice) const;
		virtual bool readFromRow(sqliteRowReference, sequence);

	private:
		static const constexpr double minHeading 	= 0.0;
		static const constexpr double maxHeading 	= 360.0;
		static const constexpr double minLatitude 	= -90.0;
		static const constexpr double maxLatitude 	= 90.0;
		static const constexpr double minLongitude 	= -180.0;
		static const constexpr double maxLongitude 	= 180.0;
		static const constexpr double minSpeed 		= 0.0;
		bool packRMC (struct minmea_sentence_rmc *frame);
		bool packGSA (struct minmea_sentence_gsa *frame);
		bool packGSV (struct minmea_sentence_gsv *frame);
		bool packGGA (struct minmea_sentence_gga *frame);
		void clearStrings (void) {
			GGA.clear();
			GSA.clear();
			VTG.clear();
			GSV.clear();
			RMC.clear();
		}
};

#endif
