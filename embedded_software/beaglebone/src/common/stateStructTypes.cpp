/******************************************************************************
 * Hackerboat Beaglebone types module
 * stateStructTypes.cpp
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
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
#include "config.h"
#include "location.hpp"
#include "logs.hpp"
#include "stateStructTypes.hpp"

#include <string>
using namespace string;

json_t *hackerboatStateClass::packTimeSpec (timespec t) {
	return json_pack("{s:i,s:i}", "tv_sec", t.tv_sec, "tv_nsec", t.tv_nsec);
}

int hackerboatStateClass::parseTimeSpec (json_t *input, timespec *t) {
	return json_unpack(input, "{s:i,s:i}", "tv_sec", &(t->tv_sec), "tv_nsec", &(t->tv_nsec));
}

json_t *gpsFixClass::pack (bool seq) {
	json_t *outTime, *output;
	outTime = packTimeSpec(this->uTime);
	output = json_pack(this->_format.c_str(),
						"uTime", outTime, 
						"latitude", latitude,
						"longitude", longitude,
						"gpsHeading", gpsHeading,
						"gpsSpeed", gpsSpeed,
						"GGA", GGA.c_str(), 
						"GSA", GSA.c_str(),
						"GSV", GSV.c_str(), 
						"VTG", VTG.c_str(),
						"RMC", RMC.c_str());
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool *gpsFixClass::parse (json_t *input, bool seq = true) {
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

json_t *waypointClass::pack (bool seq) {
	json_t *outLoc, *output;
	outLoc = location.pack();
	output = json_pack(this->_format.c_str(),
						"location", outLoc, 
						"index", index,
						"act", act);
	if (seq) json_object_set(output, "sequenceNum", json_integer(_sequenceNum));
	return output;
}

bool *waypointClass::parse (json_t *input, bool seq = true) {
	json_t *inLoc, *seqIn;
	if (json_unpack(input, this->_format.c_str(),
					"location", inLoc, 
					"index", &index,
					"act", &act)) {
		return false;
	}
	location.parse(inLoc);
	if (seq) {
		seqIn = json_object_get(input, "sequenceNum");
		if (seqIn) _sequenceNum = json_integer_value(seqIn);
	}
	free(seqIn);
	free(inLoc);
	return this->isValid();	
}
