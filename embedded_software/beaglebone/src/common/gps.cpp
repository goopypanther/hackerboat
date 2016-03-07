/******************************************************************************
 * Hackerboat Beaglebone GPS module
 * gps.cpp
 * This module houses the GPS module
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include "config.h"
#include "location.hpp"
#include "stateStructTypes.hpp"
#include "gps.hpp"

#include <string>
using namespace string;

json_t *gpsFixClass::pack (bool seq) {
	json_t *output;
	output = json_pack(this->_format.c_str(),
						"uTime", packTimeSpec(this->uTime), 
						"latitude", latitude,
						"longitude", longitude,
						"gpsHeading", gpsHeading,
						"gpsSpeed", gpsSpeed,
						"GGA", json_string(GGA.c_str()), 
						"GSA", json_string(GSA.c_str()),
						"GSV", json_string(GSV.c_str()), 
						"VTG", json_string(VTG.c_str()),
						"RMC", json_string(RMC.c_str()));
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool gpsFixClass::parse (json_t *input, bool seq = true) {
	json_t *inTime, *seqIn;
	char inGGA[LOCAL_BUF_LEN], inGSA[LOCAL_BUF_LEN], inGSV[LOCAL_BUF_LEN];
	char inVTG[LOCAL_BUF_LEN], inRMC[LOCAL_BUF_LEN];
	if (json_unpack(input, this->_format.c_str(),
					"uTime", inTime, 
					"latitude", &latitude,
					"longitude", &longitude,
					"gpsHeading", &gpsHeading,
					"gpsSpeed", &gpsSpeed,
					"GGA", inGGA, "GSA", inGSA,
					"GSV", inGSV, "VTG", inVTG,
					"RMC", inRMC)) {
		return false;
	}
	GGA = std::string(inGGA);
	GSA = std::string(inGSA);
	GSV = std::string(inGSV);
	VTG = std::string(inVTG);
	RMC = std::string(inRMC);			
	if (seq) {
		seqIn = json_object_get(input, "sequenceNum");
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}
	parseTimeSpec(inTime, &uTime);
	free(inTime);
	free(seqIn);
	return this->isValid();
}

bool gpsFixClass::isValid (void) {
	if ((GGA.length() == 0) && (GSA.length() == 0) && (GSV.length()) &&
		(VTG.length() == 0) && (RMC.length() == 0)) return false
	if (gpsSpeed < minSpeed) return false;
	if ((gpsHeading < minHeading) || (gpsHeading > maxHeading)) return false;
	if ((longitude < minLongitude) || (longitude > maxLongitude)) return false;
	if ((latitude < minLatitude) || (latitude > maxLatitude)) return false;
	return true;
}