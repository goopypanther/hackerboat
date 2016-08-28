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

using namespace std::chrono;
using namespace std::literals::chrono_literals;

AISShip::AISShip (json_t *packet) {
	recordTime = std::chrono::system_clock::now();
	parseGpsdPacket(packet);
}			

bool AISShip::parseGpsdPacket (json_t *input) {
	return false;
}

bool AISShip::parse (json_t *input) {
	bool result = true;
	int tmp;
	std::string time;
	double lat, lon;
	
	result &= this->coreParse(input);
	
	return false;
}

bool AISShip::coreParse (json_t *input) {
	AISNavStatus navstat;
	AISShipType thistype;
	AISEPFDType myepfd;
	bool result = true;
	result &= GET_VAR(mmsi);
	GET_VAR(course);
	GET_VAR(heading);
	GET_VAR(turn);
	GET_VAR(speed);
	GET_VAR(device);
	
	
	return result.
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
	bool packResult = true;
	packResult &= json_object_set_new(output, "mmsi", json_integer(this->mmsi));
	packResult &= json_object_set_new(output, "recordTime", json(packTime(this->recordTime)));
	packResult &= json_object_set_new(output, "lastTimeStamp", json(packTime(this->lastTimeStamp)));
	packResult += json_object_set_new(output, "device", json(device));
	packResult += json_object_set_new(output, "fix", this->fix.pack());
	packResult &= json_object_set_new(output, "status", json_integer(static_cast<int>(this->status)));
	packResult &= json_object_set_new(output, "turn", json_real(this->turn));
	packResult &= json_object_set_new(output, "speed", json_real(this->speed));
	packResult &= json_object_set_new(output, "course", json_real(this->course));
	packResult &= json_object_set_new(output, "heading", json_real(this->heading));
	packResult &= json_object_set_new(output, "imo", json_integer(this->imo));
	packResult &= json_object_set_new(output, "callsign", json(this->callsign));
	packResult &= json_object_set_new(output, "shipname", json(this->shipname));
	packResult &= json_object_set_new(output, "shiptype", json_integer(static_cast<int>(this->shiptype)));
	packResult &= json_object_set_new(output, "to_bow", json_integer(this->to_bow));
	packResult &= json_object_set_new(output, "to_stern", json_integer(this->to_stern));
	packResult &= json_object_set_new(output, "to_port", json_integer(this->to_port));
	packResult &= json_object_set_new(output, "to_starboard", json_integer(this->to_starboard));
	packResult &= json_object_set_new(output, "epfd", json_integer(static_cast<int>(this->epfd)));
	
	if (!packResult) {
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
	row.bind(1, packTime(recordTime));
	row.bind(2, packTime(lastTimeStamp));
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