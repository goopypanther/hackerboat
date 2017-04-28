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

#include <stdlib.h>
#include <chrono>
#include <math.h>
#include <string>
#include "gps.hpp"
#include "hal/config.h"
#include "sqliteStorage.hpp"
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "easylogging++.h"
#include "rapidjson/rapidjson.h"

using namespace rapidjson;

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

GPSFix::GPSFix(Value& packet) {
	LOG(DEBUG) << "Creating new GPSFix object from packet " << packet;
	fixValid = this->parseGpsdPacket(packet); 
}

Value GPSFix::pack () const {
	Value d;
	int packResult = 0;

	packResult += PutVar("recordTime", HackerboatState::packTime(this->recordTime), d);
	packResult += PutVar("gpsTime", HackerboatState::packTime(this->gpsTime), d);
	packResult += PutVar("mode", NMEAModeNames.get(this->mode), d);
	packResult += PutVar("device", this->device, d);
	packResult += PutVar("fix", this->fix.pack(), d);
	packResult += PutVar("track", this->track, d);
	packResult += PutVar("speed", this->speed, d);
	packResult += PutVar("alt", this->alt, d);
	packResult += PutVar("climb", this->climb, d);
	packResult += PutVar("epx", this->epx, d);
	packResult += PutVar("epy", this->epy, d);
	packResult += PutVar("epd", this->epd, d);
	packResult += PutVar("eps", this->eps, d);
	packResult += PutVar("ept", this->ept, d);
	packResult += PutVar("epv", this->epv, d);
	packResult += PutVar("epc", this->epc, d);
	packResult += PutVar("fixValid", this->fixValid, d);

	return d;
}

bool GPSFix::coreParse (Value& packet) {
	GetVar("track", this->track, packet);
	GetVar("speed", this->speed, packet);
	GetVar("alt", this->alt, packet);
	GetVar("climb", this->climb, packet);
	GetVar("ept", this->ept, packet);
	GetVar("epx", this->epx, packet);
	GetVar("epy", this->epy, packet);
	GetVar("epv", this->epv, packet);
	GetVar("epd", this->epd, packet);
	GetVar("eps", this->eps, packet);
	GetVar("epc", this->epc, packet);
	GetVar("device", this->device, packet);
	return true;
}

bool GPSFix::parseGpsdPacket (Value& input) {
	bool result = true;
	int tmp;
	std::string time;
	double lat, lon;
	
	result &= coreParse(input);
	result &= GetVar("time", time, input);
	result &= HackerboatState::parseTime(time, this->gpsTime);
	result &= GetVar("lat", lat, input);
	result &= GetVar("lon", lon, input);
	result &= GetVar("mode", tmp, input);
	if (NMEAModeNames.valid(tmp)) {
		this->mode = static_cast<NMEAModeEnum>(tmp);
	} else {
		this->mode = NMEAModeEnum::NONE;
		result = false;
	}
	if (result) {
		this->fix.lat = lat;
		this->fix.lon = lon;
	}
	this->recordTime = std::chrono::system_clock::now();
	
	LOG_IF(!result, ERROR) << "Parsing GPSFix packet input failed: ";// << input;
	
	if (result) return this->isValid();
	return false;
}

bool GPSFix::parse (Value& input) {
	Value inFix;
	bool result = true;
	std::string inTime, gpsInTime, tmp;
	
	result &= this->coreParse(input);
	result &= GetVar("fix", inFix, input);
	result &= this->fix.parse(inFix);
	result &= GetVar("recordTime", inTime, input);
	result &= GetVar("gpsTime", gpsInTime, input);
	result &= HackerboatState::parseTime(inTime, this->recordTime);
	result &= HackerboatState::parseTime(gpsInTime, this->gpsTime);
	result &= GetVar("fixValid", fixValid, input);
	result &= GetVar("mode", tmp, input);
	result &= NMEAModeNames.get(tmp, &mode);
	
	LOG_IF(!result, ERROR) << "Parsing GPSFix input failed";
	
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

void GPSFix::copy(const GPSFix *newfix) {
	this->gpsTime 	= newfix->gpsTime;
	this->mode 		= newfix->mode;
	//this->device 	= newfix->device;
	this->fix.lat 	= newfix->fix.lat;
	this->fix.lon 	= newfix->fix.lon;
	this->track		= newfix->track;
	this->speed		= newfix->speed;
	this->alt		= newfix->alt;
	this->climb		= newfix->climb;
	this->epx		= newfix->epx;
	this->epy		= newfix->epy;
	this->epd		= newfix->epd;
	this->eps		= newfix->eps;
	this->ept		= newfix->ept;
	this->epv		= newfix->epv;
	this->epc		= newfix->epc;
	this->fixValid	= newfix->fixValid;
}

void GPSFix::copy(const GPSFix &newfix) {
	this->gpsTime 	= newfix.gpsTime;
	this->mode 		= newfix.mode;
	//this->device 	= newfix.device;
	this->fix.lat 	= newfix.fix.lat;
	this->fix.lon 	= newfix.fix.lon;
	this->track		= newfix.track;
	this->speed		= newfix.speed;
	this->alt		= newfix.alt;
	this->climb		= newfix.climb;
	this->epx		= newfix.epx;
	this->epy		= newfix.epy;
	this->epd		= newfix.epd;
	this->eps		= newfix.eps;
	this->ept		= newfix.ept;
	this->epv		= newfix.epv;
	this->epc		= newfix.epc;
	this->fixValid	= newfix.fixValid;
}
