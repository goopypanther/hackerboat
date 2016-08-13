/******************************************************************************
 * Hackerboat Beaglebone AIS module
 * ais.hpp
 * This module stores AIS data 
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef AIS_H
#define AIS_H
 
#include <jansson.h>
#include "config.h"
#include <math.h>
#include <string>
#include <chrono>
#include "hackerboatRoot.hpp"
#include "location.hpp"

class aisClass : public hackerboatStateClassStorable {
	public:
		aisClass();
		
		time_point<std::chrono::system_clock> lastContact;
		int 			MMSI = -1;
		int 			IMOnumber;
		std::string 	navStatus;
		int				rateOfTurn;
		float			speedOverGround;
		locationClass	position;
		float			courseOverGround;
		float			trueHeading;
		float			trueBearing;
		float			utcSeconds;
		std::string		callSign;
		std::string		name;
		int			 	type;
		
		bool parseGPDdPacket (json_t *packet);
		
	private:
		hackerboatStateStorage *aisStorage = NULL;
	
}


#endif