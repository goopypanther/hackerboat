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
#include <time.h>
#include <math.h>
#include "minmea.h"

#include <string>

#include "gps.hpp"
#include "config.h"
#include "sqliteStorage.hpp"

gpsFixClass::gpsFixClass() {
}

json_t *gpsFixClass::pack (bool seq) const {
	json_t *output;
	output = json_pack("{s:o,s:o,s:f,s:f,s:f,s:f,s:b}",
			   "uTime", packTimeSpec(this->uTime),
			   "gpsTime", packTimeSpec(this->gpsTime),
			   "latitude", latitude,
			   "longitude", longitude,
			   "heading", gpsHeading,
			   "speed", gpsSpeed,
			   "valid", fixValid);

	if (!GGA.empty()) json_object_set_new(output, "GGA", json(GGA));
	if (!GSA.empty()) json_object_set_new(output, "GSA", json(GSA));
	if (!GSV.empty()) json_object_set_new(output, "GSV", json(GSV));
	if (!VTG.empty()) json_object_set_new(output, "VTG", json(VTG));
	if (!RMC.empty()) json_object_set_new(output, "RMC", json(RMC));

	if (seq && _sequenceNum >= 0)
		json_object_set_new(output, "sequenceNum", json_integer(_sequenceNum));

	return output;
}

bool gpsFixClass::parse (json_t *input, bool seq = true) {
	json_t *inTime, *gpsInTime;
	if (json_unpack(input, "{s:o,s:o,s:F,s:F,s:F,s:F,s:b}",
			"uTime", &inTime,
			"gpsTime", &gpsInTime,
			"latitude", &latitude,
			"longitude", &longitude,
			"heading", &gpsHeading,
			"speed", &gpsSpeed,
			"valid", &fixValid)) {
		return false;
	}
	if ((!::parse(inTime, &uTime)) && (!::parse(inGPSTime, &gpsTime))) {
		return false;
	}

	json_t *tmp;

#define GET_OPTIONAL(var) if( (tmp = json_object_get(input, #var )) != NULL ) { if (!::parse(tmp, &var)) return false; } else { var.clear(); }
	GET_OPTIONAL(GGA);
	GET_OPTIONAL(GSA);
	GET_OPTIONAL(GSV);
	GET_OPTIONAL(VTG);
	GET_OPTIONAL(RMC);
#undef GET_OPTIONAL

	if (seq) {
		tmp = json_object_get(input, "sequenceNum");
		if (!json_is_integer(tmp))
			return false;
		_sequenceNum = json_integer_value(tmp);
	}

	return this->isValid();
}

bool gpsFixClass::isValid (void) const {
	if ((GGA.length() == 0) && (GSA.length() == 0) && (GSV.length()) &&
	    (VTG.length() == 0) && (RMC.length() == 0))
		return false;
	if (gpsSpeed < minSpeed) return false;
	if ((gpsHeading < minHeading) || (gpsHeading > maxHeading)) return false;
	if ((longitude < minLongitude) || (longitude > maxLongitude)) return false;
	if ((latitude < minLatitude) || (latitude > maxLatitude)) return false;
	return fixValid;
}

hackerboatStateStorage &gpsFixClass::storage() {
	static hackerboatStateStorage *gpsStorage;

	if (!gpsStorage) {
		gpsStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(GPS_DB_FILE),
							"GPS_FIX",
							{ { "time", "REAL" },
							  { "latitude", "REAL" },
							  { "longitude", "REAL" },
							  { "heading", "REAL" },
							  { "speed", "REAL" },
							  { "sentence", "TEXT" },
							  { "data", "TEXT" } });
		gpsStorage->createTable();
	}

	return *gpsStorage;
}

bool gpsFixClass::fillRow(sqliteParameterSlice row) const {
	row.assertWidth(7);
	row.bind(0, (double)uTime.tv_sec + 1e-9 * uTime.tv_nsec);
	row.bind(1, latitude);
	row.bind(2, longitude);
	row.bind(3, gpsHeading);
	row.bind(4, gpsSpeed);

	if (GGA.length()) {
		row.bind(5, "GGA");
		row.bind(6, GGA);
	} else if (GSA.length()) {
		row.bind(5, "GSA");
		row.bind(6, GSA);
	} else if (GSV.length()) {
		row.bind(5, "GSV");
		row.bind(6, GSV);
	} else if (VTG.length()) {
		row.bind(5, "VTG");
		row.bind(6, VTG);
	} else if (RMC.length()) {
		row.bind(5, "RMC");
		row.bind(6, RMC);
	} else {
		row.bind_null(5);
		row.bind_null(6);
	}

	return true;
}

bool gpsFixClass::readFromRow(sqliteRowReference row, sequence seq) {
	#warning Implement GPS readFromRow()
	return false;
}

bool gpsFixClass::readSentence (std::string sentence) {
	clock_gettime(CLOCK_REALTIME, &uTime);
	switch(minmea_sentence_id(sentence.c_str(), true)) {
		case MINMEA_SENTENCE_RMC:
			struct minmea_sentence_rmc frame;
			if (minmea_parse_rmc(&frame, sentence.c_str())) {
				clearStrings();
				RMC = sentence;
				return packRMC(&frame);
			}
			break;
		case MINMEA_SENTENCE_GGA:
			struct minmea_sentence_gga frame;
			if (minmea_parse_gga(&frame, sentence.c_str())) {
				clearStrings();
				GGA = sentence;
				return packRMC(&frame);
			}
			break;
		case MINMEA_SENTENCE_GSA:
			struct minmea_sentence_gsa frame;
			if (minmea_parse_gsa(&frame, sentence.c_str())) {
				clearStrings();
				GSA = sentence;
				return packRMC(&frame);
			}
			break;
		case MINMEA_SENTENCE_GSV:
			struct minmea_sentence_gsv frame;
			if (minmea_parse_gsv(&frame, sentence.c_str())) {
				clearStrings();
				GSV = sentence;
				return packRMC(&frame);
			}
			break;
		default:
			break;
	}
	return false;
}

bool gpsFixClass::packRMC (struct minmea_sentence_rmc *frame) {
	if (frame->valid) {
		this->fixValid = true;
		this->longtitude = minmea_tofloat(frame->longitude);
		this->latitude = minmea_tofloat(frame->latitude);
		this->gpsHeading = minmea_tofloat(frame->course);
		this->gpsSpeed = minmea_tofloat(frame->speed);
		minmea_gettime(&gpsTime, frame->date, frame->time);
	} return false;
}

double		latitude;				/**< Latitude of last fix */
		double		longitude;				/**< Longitude of last fix */
		double		gpsHeading;				/**< True heading, according to GPS */
		double		gpsSpeed;				/**< Speed over the ground */
		bool 		fixValid;				/**< Checks whether this fix is valid or not */				
		

bool gpsFixClass::packGSA (struct minmea_sentence_rmc *frame) {
	return true;
}

bool gpsFixClass::packGSV (struct minmea_sentence_rmc *frame) {
	return true;
}

bool gpsFixClass::packGGA (struct minmea_sentence_rmc *frame) {
	if ((frame->fix_quality > 0) && (frame->fix_quality < 4)) {
		this->fixValid = true;
		this->longtitude = minmea_tofloat(frame->longitude);
		this->latitude = minmea_tofloat(frame->latitude);
	} else return false;
}