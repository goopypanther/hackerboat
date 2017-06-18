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

#include "rapidjson/rapidjson.h"

#include <stdlib.h>
#include <math.h>
#include <string>
#include <chrono>
#include <stdexcept>
#include "hackerboatRoot.hpp"
#include "location.hpp"

#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "ais.hpp"
#include "easylogging++.h"
#include "configuration.hpp"

#define METERS_PER_KNOT		(1852)
#define SECONDS_PER_HOUR	(3600)

using namespace std::chrono;
using namespace std::literals::chrono_literals;
using namespace rapidjson;

const std::string AISBase::msgClass = "AIS";

AISShip::AISShip (Value& packet) {
	VLOG(2) << "Creating new AIS object";
	recordTime = std::chrono::system_clock::now();
	parseGpsdPacket(packet);
}			

bool AISShip::parseGpsdPacket (Value& input) {
	bool result = true;
	root.CopyFrom(input, root.GetAllocator());
	
	result = coreParse(input);
	this->lastTimeStamp = std::chrono::system_clock::now();
	if (input.HasMember("lat") && input["lat"].IsDouble() &&
		input.HasMember("lon") && input["lon"].IsDouble()) {
			this->fix.lat = input["lat"].GetDouble();
			this->fix.lon = input["lon"].GetDouble();
	} else result = false;
	
	LOG_IF((!result), ERROR) << "Parsing AIS input from gpsd failed: " << input;
	
	if (result) return this->isValid();
	return result;
}

bool AISShip::parse (Value& input) {
	bool result = true;
	std::string myTime, lastTime;
	
	result &= this->coreParse(input);
	if (input.HasMember("fix")) {
		result &= this->fix.parse(input["fix"]);
	} else result = false;
	if (input.HasMember("recordTime") && input["recordTime"].IsString()) {
		result &= parseTime(input["recordTime"].GetString(), this->recordTime);
	} else result = false;
	if (input.HasMember("lastTimeStamp") && input["lastTimeStamp"].IsString()) {
		result &= parseTime(input["lastTimeStamp"].GetString(), this->lastTimeStamp);
	} else result = false;
	
	LOG_IF((!result), ERROR) << "Parsing AIS input failed: " << input;
	if (result) return this->isValid();
	return result;
}

bool AISShip::coreParse (Value& input) {
	bool result = true;
	int tmp;
	
	result &= GetVar("mmsi", 		this->mmsi, 	input);
	result &= GetVar("course", 		this->course, 	input);
	result &= GetVar("heading", 	this->heading, 	input);
	result &= GetVar("turn", 		this->turn, 	input);
	result &= GetVar("speed", 		this->speed, 	input);
	result &= GetVar("device", 		this->device, 	input);
	result &= GetVar("imo", 		this->imo, 		input);
	result &= GetVar("callsign", 	this->callsign, input);
	result &= GetVar("shipname", 	this->shipname, input);
	result &= GetVar("to_bow", 		this->to_bow, 	input);
	result &= GetVar("to_starboard",this->to_starboard, input);
	result &= GetVar("to_port", 	this->to_port, 	input);
	result &= GetVar("to_stern", 	this->to_stern, input);
	
	GetVar("status", tmp, input);
	try {
		this->status = static_cast<AISNavStatus>(tmp);
	} catch (...) {
		this->status = AISNavStatus::UNDEFINED;
		result = false;
	}
	GetVar("shiptype", tmp, input);
	try {
		this->shiptype = static_cast<AISShipType>(tmp);
	} catch (...) {
		this->shiptype = AISShipType::UNAVAILABLE;
		result = false;
	}
	GetVar("epfd", tmp, input);
	try {
		this->epfd = static_cast<AISEPFDType>(tmp);
	} catch (...) {
		this->epfd = AISEPFDType::UNDEFINED;
		result = false;
	}
	
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
	auto timeout = Conf::get()->aisMaxTime();
	if ((!this->isValid()) || 
		(current.isValid() && (fix.distance(current) > Conf::get()->aisMaxDistance())) ||
		((std::chrono::system_clock::now() - lastTimeStamp) > timeout)) {
			LOG(DEBUG) << "Trimming target " << this->mmsi;
			LOG(DEBUG) << "Trimmed target " << *this;
			return true;
		} else return false;
	return false;
}

Value AISShip::pack () const {
	Value d;
	int packResult = 0;

	packResult += PutVar("mmsi", this->mmsi, d);
	packResult += PutVar("recordTime", HackerboatState::packTime(this->recordTime), d);
	packResult += PutVar("lastTimeStamp", HackerboatState::packTime(this->lastTimeStamp), d);
	packResult += PutVar("device", this->device, d);
	packResult += PutVar("fix", this->fix.pack(), d);
	packResult += PutVar("status", static_cast<int>(this->status), d);
	packResult += PutVar("turn", this->turn, d);
	packResult += PutVar("speed", this->speed, d);
	packResult += PutVar("course", this->course, d);
	packResult += PutVar("heading", this->heading, d);
	packResult += PutVar("imo", this->heading, d);
	packResult += PutVar("callsign", this->callsign, d);
	packResult += PutVar("shipname", this->shipname, d);
	packResult += PutVar("shiptype", static_cast<int>(this->shiptype), d);
	packResult += PutVar("to_bow", this->to_bow, d);
	packResult += PutVar("to_stern", this->to_stern, d);
	packResult += PutVar("to_port", this->to_port, d);
	packResult += PutVar("to_starboard", this->to_starboard, d);
	packResult += PutVar("epfd", static_cast<int>(this->epfd), d);
	
	return d;
}

bool AISShip::isValid () const {
	bool result = true;
	auto timeout = Conf::get()->aisMaxTime();
	result &= (mmsi > 0);
	result &= (fix.isValid());
	result &= ((std::chrono::system_clock::now() - (this->lastTimeStamp)) < timeout);
	return result;
}

bool AISShip::merge(AISShip& other) {
	AISShip *a,*b;
	if (this->mmsi != other.mmsi) return false;		// Can only merge contacts with the same MMSI
	if (this->lastTimeStamp > other.lastTimeStamp) {	// make the newer one dominant
		a = this; 
		b = &other;
	} else {
		a = &other;
		b = this;
	}
	this->recordTime 	= std::chrono::system_clock::now();
	this->lastTimeStamp = a->lastTimeStamp;
	LOG(DEBUG) << "Merging AIS contacts with MMSI: " << this->mmsi;
	
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

void AISShip::copy (const AISShip& c) {
	this->fix = c.fix;
	this->device = c.device;
	this->status = c.status;
	this->turn = c.turn;
	this->speed = c.speed;
	this->course = c.course;
	this->heading = c.heading;
	this->imo = c.imo;
	this->callsign = c.callsign;
	this->shipname = c.shipname;
	this->shiptype = c.shiptype;
	this->to_bow = c.to_bow;
	this->to_starboard = c.to_starboard;
	this->to_port = c.to_port;
	this->to_stern = c.to_stern;
	this->epfd = c.epfd;
}

std::ostream& operator<< (std::ostream& stream, const AISShip& state) {
	stream << state.pack();
	return stream;
}
