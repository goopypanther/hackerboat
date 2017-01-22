/******************************************************************************
 * Hackerboat Beaglebone GPS module
 * gps.cpp
 * This module houses the GPSFix type
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include <stdlib.h>
#include <chrono>
#include <math.h>
#include <string>
#include "gps.hpp"
#include "hal/config.h"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "easylogging++.h"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

const EnumNameTable<NMEAModeEnum> GPSFix::NMEAModeNames = {
	"None", 
	"NoFix", 
	"Fix2D", 
	"Fix3D"
};

GPSFix::GPSFix() {
	LOG(DEBUG) << "Creating new blank GPSFix object";
	recordTime = std::chrono::system_clock::now();
	gpsTime = recordTime;
	fix = Location(47.560644, -122.338816);	// location of HBL
	fixValid = false;
}

GPSFix::GPSFix(json_t *packet) {
	LOG(DEBUG) << "Creating new GPSFix object from packet " << packet;
	fixValid = this->parseGpsdPacket(packet); 
}

json_t *GPSFix::pack () const {
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "recordTime", json(HackerboatState::packTime(this->recordTime)));
	packResult += json_object_set_new(output, "gpsTime", json(HackerboatState::packTime(this->gpsTime)));
	packResult += json_object_set_new(output, "mode", json(NMEAModeNames.get(mode)));
	packResult += json_object_set_new(output, "device", json(device));
	packResult += json_object_set_new(output, "fix", this->fix.pack());
	packResult += json_object_set_new(output, "track", json_real(track));
	packResult += json_object_set_new(output, "speed", json_real(speed));
	packResult += json_object_set_new(output, "alt", json_real(track));
	packResult += json_object_set_new(output, "climb", json_real(speed));
	packResult += json_object_set_new(output, "epx", json_real(epx));
	packResult += json_object_set_new(output, "epy", json_real(epy));
	packResult += json_object_set_new(output, "epd", json_real(epd));
	packResult += json_object_set_new(output, "eps", json_real(eps));
	packResult += json_object_set_new(output, "ept", json_real(epy));
	packResult += json_object_set_new(output, "epv", json_real(epd));
	packResult += json_object_set_new(output, "epc", json_real(eps));
	packResult += json_object_set_new(output, "fixValid", json_boolean(fixValid));

	if (packResult != 0) {
		if (output) {
			//LOG(ERROR) << "GPSFix pack failed: " << output;
		} else {
			//LOG(WARNING) << "GPSFix pack failed, no output";
		}
		json_decref(output);
		return NULL;
	}
	return output;
}

bool GPSFix::coreParse (json_t *input) {
	GET_VAR(track);
	GET_VAR(speed);
	GET_VAR(alt);
	GET_VAR(climb);
	GET_VAR(ept);
	GET_VAR(epx);
	GET_VAR(epy);
	GET_VAR(epv);
	GET_VAR(epd);
	GET_VAR(eps);
	GET_VAR(epc);
	GET_VAR(device);
	return true;
}

bool GPSFix::parseGpsdPacket (json_t *input) {
	json_t* gpsMode;
	bool result = true;
	int tmp;
	std::string time;
	double lat, lon;
	
	result &= coreParse(input);
	result &= GET_VAR(time);
	this->gpsTime = std::chrono::system_clock::now();
	result &= GET_VAR(lat);
	result &= GET_VAR(lon);
	gpsMode = json_object_get(input, "mode");
	result &= ::parse(gpsMode, &tmp);
	if (NMEAModeNames.valid(tmp)) {
		this->mode = static_cast<NMEAModeEnum>(tmp);
	} else {
		this->mode = NMEAModeEnum::NONE;
		result = false;
	}
	this->fix.lat = lat;
	this->fix.lon = lon;
	this->recordTime = std::chrono::system_clock::now();
	json_decref(gpsMode);
	
	LOG_IF((!result && input), ERROR) << "Parsing GPSFix packet input failed: " << input;
	LOG_IF(!input, WARNING) << "Attempted to parse NULL JSON in GPSFix.parseGpsdPacket()";
	
	if (result) return this->isValid();
	return false;
}

bool GPSFix::parse (json_t *input) {
	json_t *inFix;
	bool result = true;
	std::string inTime, gpsInTime, tmp;
	
	inFix = json_object_get(input, "fix");
	result &= ::parse(json_object_get(input, "recordTime"), &inTime);
	result &= ::parse(json_object_get(input, "gpsTime"), &gpsInTime);
	result &= HackerboatState::parseTime(inTime, this->recordTime);
	result &= HackerboatState::parseTime(gpsInTime, this->gpsTime);
	result &= this->coreParse(input);
	result &= GET_VAR(fixValid);
	result &= fix.parse(inFix);
	result &= ::parse(json_object_get(input, "mode"), &tmp);
	result &= NMEAModeNames.get(tmp, &mode);
	json_decref(inFix);
	
	LOG_IF((!result && input), ERROR) << "Parsing GPSFix input failed";
	LOG_IF(!input, WARNING) << "Attempted to parse NULL JSON in GPSFix.parse()";
	
	if (result) return this->isValid();
	return false;
}

bool GPSFix::isValid (void) const {

	if (speed < 0) {
		LOG(DEBUG) << "Invalid GPS speed: " << to_string(speed);
		return false;
	}
	if ((track < -180) || (track > 360)) {
		LOG(DEBUG) << "Invalid GPS course " << to_string(track);
		return false;
	}
	return this->fix.isValid();
}

HackerboatStateStorage &GPSFix::storage() {

	if (!gpsStorage) {
		LOG(DEBUG) << "Creating GPSFix database table";
		gpsStorage = new HackerboatStateStorage(HackerboatStateStorage::databaseConnection(GPS_DB_FILE),
							"GPS_FIX",
							{ { "recordTime", "TEXT" },
							  { "gpsTime", "TEXT"},
							  { "lat", "REAL" },
							  { "lon", "REAL" },
							  { "track", "REAL" },
							  { "speed", "REAL" },
							  { "alt", "REAL" },
							  { "climb", "REAL" },
							  { "ept", "REAL" },
							  { "epx", "REAL" },
							  { "epy", "REAL" },
							  { "epv", "REAL" },
							  { "epd", "REAL" },
							  { "eps", "REAL" },
							  { "epc", "REAL" },
							  { "device", "TEXT" },
							  { "fixValid", "INTEGER" } });
		gpsStorage->createTable();
	}

	return *gpsStorage;
}

bool GPSFix::fillRow(SQLiteParameterSlice row) const {
	row.assertWidth(17);
	LOG_EVERY_N(10, DEBUG) << "Storing GPSFix object to the database" << *this;
	LOG(DEBUG) << "Storing GPSFix object to the database" << *this;
	row.bind(0, HackerboatState::packTime(recordTime));
	row.bind(1, HackerboatState::packTime(gpsTime));
	row.bind(2, fix.lat);
	row.bind(3, fix.lon);
	row.bind(4, track);
	row.bind(5, speed);
	row.bind(6, alt);
	row.bind(7, climb);
	row.bind(8, ept);
	row.bind(9, epx);
	row.bind(10, epy);
	row.bind(11, epv);
	row.bind(12, epd);
	row.bind(13, eps);
	row.bind(14, epc);
	row.bind(15, device);
	row.bind(16, fixValid);

	return true;
}

bool GPSFix::readFromRow(SQLiteRowReference row, sequence seq) {
	bool result = true;
	_sequenceNum = seq;
	row.assertWidth(17);
	
	std::string recordTimeStr = row.string_field(0);
	result &= HackerboatState::parseTime(recordTimeStr, this->recordTime);
	std::string gpsTimeStr = row.string_field(1);
	result &= HackerboatState::parseTime(gpsTimeStr, this->gpsTime);
	this->fix.lat = row.double_field(2);
	this->fix.lon = row.double_field(3);
	this->track = row.double_field(4);
	this->speed = row.double_field(5);
	this->alt = row.double_field(6);
	this->climb = row.double_field(7);
	this->ept = row.double_field(8);
	this->epx = row.double_field(9);
	this->epy = row.double_field(10);
	this->epv = row.double_field(11);
	this->epd = row.double_field(12);
	this->eps = row.double_field(13);
	this->epc = row.double_field(14);
	this->device = row.string_field(15);
	this->fixValid = row.bool_field(16);
	LOG(DEBUG) << "Populated GPSFix object from DB " << *this;
	
	if (fixValid && result) return this->isValid();
	return false;
}
