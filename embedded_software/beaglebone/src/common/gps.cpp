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
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include "gps.hpp"
#include "config.h"
#include "sqliteStorage.hpp"

gpsFixClass::gpsFixClass() {
	clock_gettime(CLOCK_REALTIME, &(uTime));
	gpsTime = uTime;
	latitude = 47.560644;			// latitude of HBL
	longitude = -122.338816;		// longitude of HBL
	gpsHeading = 0;
	gpsSpeed = 0;
	fixValid = false;
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
			   "fixValid", fixValid);

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
			"fixValid", &fixValid)) {
		return false;
	}
	if ((!::parse(inTime, &uTime)) || (!::parse(gpsInTime, &gpsTime))) {
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
	return true;
}

hackerboatStateStorage &gpsFixClass::storage() {
	static hackerboatStateStorage *gpsStorage;

	if (!gpsStorage) {
		gpsStorage = new hackerboatStateStorage(hackerboatStateStorage::databaseConnection(GPS_DB_FILE),
							"GPS_FIX",
							{ { "time", "REAL" },
							  { "gpsTime", "REAL"},
							  { "latitude", "REAL" },
							  { "longitude", "REAL" },
							  { "heading", "REAL" },
							  { "speed", "REAL" },
							  { "fixValid", "INTEGER" } });
		gpsStorage->createTable();
	}

	return *gpsStorage;
}

bool gpsFixClass::fillRow(sqliteParameterSlice row) const {
	row.assertWidth(7);
	row.bind(0, (double)uTime.tv_sec + 1e-9 * uTime.tv_nsec);
	row.bind(1, (double)gpsTime.tv_sec + 1e-9 * gpsTime.tv_nsec);
	row.bind(2, latitude);
	row.bind(3, longitude);
	row.bind(4, gpsHeading);
	row.bind(5, gpsSpeed);
	row.bind(6, fixValid);

	return true;
}

bool gpsFixClass::readFromRow(sqliteRowReference row, sequence seq) {
	_sequenceNum = seq;
	row.assertWidth(7);
	double timestamp = row.double_field(0);
	uTime.tv_sec = floor(timestamp);
	uTime.tv_nsec = ( timestamp - floor(timestamp) ) * 1e9;
	timestamp = row.double_field(1);
	gpsTime.tv_sec = floor(timestamp);
	gpsTime.tv_nsec = ( timestamp - floor(timestamp) ) * 1e9;
	latitude = row.double_field(2);
	longitude = row.double_field(3);
	gpsHeading = row.double_field(4);
	gpsSpeed = row.double_field(5);
	fixValid = row.bool_field(6);
	return fixValid;
}

bool gpsFixClass::readSentence (std::string sentence) {
	clock_gettime(CLOCK_REALTIME, &uTime);
	switch(minmea_sentence_id(sentence.c_str(), true)) {
		case MINMEA_SENTENCE_RMC:
			struct minmea_sentence_rmc frame_rmc;
			if (minmea_parse_rmc(&frame_rmc, sentence.c_str())) {
				clearStrings();
				RMC = sentence;
				return packRMC(&frame_rmc);
			}
			break;
		case MINMEA_SENTENCE_GGA:
			struct minmea_sentence_gga frame_gga;
			if (minmea_parse_gga(&frame_gga, sentence.c_str())) {
				clearStrings();
				GGA = sentence;
				return packGGA(&frame_gga);
			}
			break;
		case MINMEA_SENTENCE_GSA:
			struct minmea_sentence_gsa frame_gsa;
			if (minmea_parse_gsa(&frame_gsa, sentence.c_str())) {
				clearStrings();
				GSA = sentence;
				return packGSA(&frame_gsa);
			}
			break;
		case MINMEA_SENTENCE_GSV:
			struct minmea_sentence_gsv frame_gsv;
			if (minmea_parse_gsv(&frame_gsv, sentence.c_str())) {
				clearStrings();
				GSV = sentence;
				return packGSV(&frame_gsv);
			}
			break;
		default:
			break;
	}
	return false;
}

bool gpsFixClass::packRMC (struct minmea_sentence_rmc *frame) {
	if (frame->valid) {
		this->longitude = minmea_tofloat(&(frame->longitude));
		this->latitude = minmea_tofloat(&(frame->latitude));
		this->gpsHeading = minmea_tofloat(&(frame->course));
		this->gpsSpeed = minmea_tofloat(&(frame->speed));
		minmea_gettime(&gpsTime, &(frame->date), &(frame->time));
		this->fixValid = this->isValid();
	} return false;
}

bool gpsFixClass::packGSA (struct minmea_sentence_gsa *frame) {
	return true;
}

bool gpsFixClass::packGSV (struct minmea_sentence_gsv *frame) {
	return true;
}

bool gpsFixClass::packGGA (struct minmea_sentence_gga *frame) {
	if ((frame->fix_quality > 0) && (frame->fix_quality < 4)) {
		this->longitude = minmea_tofloat(&(frame->longitude));
		this->latitude = minmea_tofloat(&(frame->latitude));
		this->fixValid = this->isValid();
		return true;
	} else return false;
}

int gpsFixClass::openGPSserial (void) {
	struct termios gps_attrib;
	
	gps_fd = open(GNSS_TTY, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (gps_fd == -1) return gps_fd;	
	if (tcgetattr(gps_fd, &gps_attrib) < 0) {
		closeGPSserial();
		return gps_fd;
	}
	cfsetospeed(&gps_attrib, GNSS_BPS);
	cfsetospeed(&gps_attrib, GNSS_BPS);
	
	if (tcsetattr(gps_fd, TCSANOW, &gps_attrib) != 0) {
		closeGPSserial();
    }
	
	return gps_fd;
}

void gpsFixClass::closeGPSserial (void) {
	close(gps_fd);
	gps_fd = -1;
}