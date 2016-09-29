/******************************************************************************
 * Hackerboat Beaglebone AIS module
 * ais.cpp
 * This module stores AIS data
 * Navigation formulas from Ed Williams' Aviation Formulary
 * http://williams.best.vwh.net/avform.htm
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include "hal/config.h"
#include <stdlib.h>
#include <math.h>
#include <string>
#include <chrono>
#include <stdexcept>
#include "hackerboatRoot.hpp"
#include "location.hpp"
#include "hal/config.h"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "ais.hpp"

#define METERS_PER_KNOT		(1852)
#define SECONDS_PER_HOUR	(3600)
#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)

using namespace std::chrono;
using namespace std::literals::chrono_literals;

const std::string AISBase::msgClass = "AIS";

AISShip::AISShip (json_t *packet) {
	recordTime = std::chrono::system_clock::now();
	parseGpsdPacket(packet);
}			

bool AISShip::parseGpsdPacket (json_t *input) {
	bool result = true;
	std::string time;
	double lat, lon;
	
	result &= coreParse(input);
	result &= GET_VAR(time);
	this->lastTimeStamp = std::chrono::system_clock::now();
	result &= GET_VAR(lat);
	result &= GET_VAR(lon);
	
	if (result) return this->isValid();
	return result;
}

bool AISShip::parse (json_t *input) {
	json_t* inFix;
	bool result = true;
	std::string myTime, lastTime;
	
	inFix = json_object_get(input, "fix");
	result &= this->coreParse(input);
	result &= ::parse(json_object_get(input, "recordTime"), &myTime);
	result &= ::parse(json_object_get(input, "lastTimeStamp"), &lastTime);
	result &= parseTime(myTime, this->recordTime);
	result &= parseTime(lastTime, this->lastTimeStamp);
	result &= fix.parse(inFix);
	json_decref(inFix);
	
	if (result) return this->isValid();
	return result;
}

bool AISShip::coreParse (json_t *input) {
	bool result = true;
	json_t* tmpObj;
	int tmpInt;
	
	result &= GET_VAR(mmsi);
	GET_VAR(course);
	GET_VAR(heading);
	GET_VAR(turn);
	GET_VAR(speed);
	GET_VAR(device);
	GET_VAR(imo);
	GET_VAR(callsign);
	GET_VAR(shipname);
	GET_VAR(to_bow);
	GET_VAR(to_starboard);
	GET_VAR(to_stern);
	GET_VAR(to_port);
	tmpObj = json_object_get(input, "status");
	if (::parse(tmpObj, &tmpInt)) {
		try {
			this->status = static_cast<AISNavStatus>(tmpInt);
		} catch (...) {
			status = AISNavStatus::UNDEFINED;
		}
	}
	json_decref(tmpObj);
	tmpObj = json_object_get(input, "shiptype");
	if (::parse(tmpObj, &tmpInt)) {
		try {
			this->shiptype = static_cast<AISShipType>(tmpInt);
		} catch (...) {
			this->shiptype = AISShipType::UNAVAILABLE;
		}
	}
	json_decref(tmpObj);
	tmpObj = json_object_get(input, "epfd");
	if (::parse(tmpObj, &tmpInt)) {
		try {
			this->epfd = static_cast<AISEPFDType>(tmpInt);
		} catch (...) {
			this->epfd = AISEPFDType::UNDEFINED;
		}
	}
	json_decref(tmpObj);
	
	return result;
}

Location AISShip::project () {
	return project(std::chrono::system_clock::now());
}	
					
Location AISShip::project (sysclock t) {
	auto d = t - this->lastTimeStamp;
	double distance = speed*(std::chrono::duration_cast<std::chrono::seconds>(d).count()/SECONDS_PER_HOUR)*METERS_PER_KNOT;	// distance traveled, in meters
	TwoVector projection;
	projection.angleDeg(course);
	projection.mag(distance);
	return fix.project(projection);
}
			
bool AISShip::prune (Location& current) {
	auto timeout = AIS_MAX_TIME;
	if ((!this->isValid()) || 
		(current.isValid() && (fix.distance(current) > AIS_MAX_DISTANCE)) ||
		((std::chrono::system_clock::now() - lastTimeStamp) > timeout)) {
		removeEntry();
		return true;
	} else return false;
	return false;
}

json_t *AISShip::pack () const {
	json_t* output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "mmsi", json_integer(this->mmsi));
	packResult += json_object_set_new(output, "recordTime", json(HackerboatState::packTime(this->recordTime)));
	packResult += json_object_set_new(output, "lastTimeStamp", json(HackerboatState::packTime(this->lastTimeStamp)));
	packResult += json_object_set_new(output, "device", json(device));
	packResult += json_object_set_new(output, "fix", this->fix.pack());
	packResult += json_object_set_new(output, "status", json_integer(static_cast<int>(this->status)));
	packResult += json_object_set_new(output, "turn", json_real(this->turn));
	packResult += json_object_set_new(output, "speed", json_real(this->speed));
	packResult += json_object_set_new(output, "course", json_real(this->course));
	packResult += json_object_set_new(output, "heading", json_real(this->heading));
	packResult += json_object_set_new(output, "imo", json_integer(this->imo));
	packResult += json_object_set_new(output, "callsign", json(this->callsign));
	packResult += json_object_set_new(output, "shipname", json(this->shipname));
	packResult += json_object_set_new(output, "shiptype", json_integer(static_cast<int>(this->shiptype)));
	packResult += json_object_set_new(output, "to_bow", json_integer(this->to_bow));
	packResult += json_object_set_new(output, "to_stern", json_integer(this->to_stern));
	packResult += json_object_set_new(output, "to_port", json_integer(this->to_port));
	packResult += json_object_set_new(output, "to_starboard", json_integer(this->to_starboard));
	packResult += json_object_set_new(output, "epfd", json_integer(static_cast<int>(this->epfd)));
	
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool AISShip::isValid () const {
	bool result = true;
	auto timeout = AIS_MAX_TIME;
	result &= (mmsi > 0);
	result &= (fix.isValid());
	result &= ((std::chrono::system_clock::now() - (this->lastTimeStamp)) < timeout);
	return result;
}

HackerboatStateStorage& AISShip::storage() {
	if (!aisShipStorage) {
		aisShipStorage = new HackerboatStateStorage(HackerboatStateStorage::databaseConnection(AIS_DB_FILE),
							"AIS_SHIP",
							{ { "mmsi", "INTEGER"},
							  { "recordTime", "TEXT" },
							  { "lastTimeStamp", "TEXT"},
							  { "lat", "REAL" },
							  { "lon", "REAL" },
							  { "device", "TEXT" },
							  { "status", "INTEGER" },
							  { "turn", "REAL" },
							  { "speed", "REAL" },
							  { "course", "REAL" },
							  { "heading", "REAL" },
							  { "imo", "INTEGER" },
							  { "callsign", "TEXT" },
							  { "shipname", "TEXT" },
							  { "shiptype", "INTEGER" },
							  { "to_bow", "INTEGER" },
							  { "to_stern", "INTEGER" },
							  { "to_port", "INTEGER" },
							  { "to_starboard", "INTEGER" },
							  { "epfd", "INTEGER" } });
		aisShipStorage->createTable();
	}

	return *aisShipStorage;
}

bool AISShip::fillRow(SQLiteParameterSlice row) const {
	row.assertWidth(20);
	row.bind(0, mmsi);
	row.bind(1, HackerboatState::packTime(recordTime));
	row.bind(2, HackerboatState::packTime(lastTimeStamp));
	row.bind(3, fix.lat);
	row.bind(4, fix.lon);
	row.bind(5, device);
	row.bind(6, static_cast<int>(status));
	row.bind(7, turn);
	row.bind(8, speed);
	row.bind(9, course);
	row.bind(10, heading);
	row.bind(11, imo);
	row.bind(12, callsign);
	row.bind(13, shipname);
	row.bind(14, static_cast<int>(shiptype));
	row.bind(15, to_bow);
	row.bind(16, to_stern);
	row.bind(17, to_port);
	row.bind(18, to_starboard);
	row.bind(19, static_cast<int>(epfd));
	
	return true;
}

bool AISShip::readFromRow(SQLiteRowReference row, sequence seq) {
	bool result = true;
	row.assertWidth(20);
	
	this->mmsi = row.int64_field(0);
	std::string recordTimeStr = row.string_field(1);
	result &= parseTime(recordTimeStr, this->recordTime);
	std::string aisTimeStr = row.string_field(2);
	result &= parseTime(aisTimeStr, this->lastTimeStamp);
	this->fix.lat = row.double_field(3);
	this->fix.lon = row.double_field(4);
	this->device = row.string_field(5);
	this->status = static_cast<AISNavStatus>(row.int64_field(6));
	this->turn = row.double_field(7);
	this->speed = row.double_field(8);
	this->course = row.double_field(9);
	this->heading = row.double_field(10);
	this->imo = row.int64_field(11);
	this->callsign = row.string_field(12);
	this->shipname = row.string_field(13);
	this->shiptype = static_cast<AISShipType>(row.int64_field(14));
	this->to_bow = row.int64_field(15);
	this->to_stern = row.int64_field(16);
	this->to_port = row.int64_field(17);
	this->to_starboard = row.int64_field(18);
	this->epfd = static_cast<AISEPFDType>(row.int64_field(19));
	
	return this->isValid();
}

bool AISShip::removeEntry () {
	// implementation postponed -- we can allow the DB to grow 
	return false;
}

bool AISShip::merge(AISShip* other) {
	AISShip *a,*b;
	if (this->mmsi != other->mmsi) return false;		// Can only merge contacts with the same MMSI
	if (this->lastTimeStamp > other->lastTimeStamp) {	// make the newer one dominant
		a = this; 
		b = other;
	} else {
		a = other;
		b = this;
	}
	this->recordTime 	= std::chrono::system_clock::now();
	this->lastTimeStamp = a->lastTimeStamp;
	
	// Check the newer version for validity and if it's valid, use it; otherwise, use the older one
	this->fix 			= (a->fix.isValid()) ? a->fix : b->fix;
	this->device		= (a->device != "") ? a->device : b->device;
	this->status 		= (a->status != AISNavStatus::UNDEFINED) ? b->status : a->status;
	this->turn			= (std::isnormal(a->turn)) ? a->turn : b->turn;
	this->speed			= (std::isnormal(a->speed)) ? a->speed : b->speed;
	this->course		= (std::isnormal(a->course)) ? a->course : b->course;
	this->heading		= (std::isnormal(a->heading)) ? a->heading : b->heading;
	this->imo			= (a->imo >= 0) ? a->imo : b->imo;
	this->callsign		= (a->callsign != "") ? a->callsign : b->callsign;
	this->shipname		= (a->shipname != "") ? a->shipname : b->shipname;
	this->shiptype		= (a->shiptype != AISShipType::UNAVAILABLE) ? a->shiptype : b->shiptype;
	this->to_bow		= (a->to_bow >= 0) ? a->to_bow : b->to_bow;
	this->to_stern		= (a->to_stern >= 0) ? a->to_stern : b->to_stern;
	this->to_port		= (a->to_port >= 0) ? a->to_port : b->to_port;
	this->to_starboard	= (a->to_starboard >= 0) ? a->to_starboard : b->to_starboard;
	this->epfd			= (a->epfd != AISEPFDType::UNDEFINED) ? a->epfd : b->epfd;
	
	return true;
}