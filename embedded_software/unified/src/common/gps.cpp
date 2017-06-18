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
	root.CopyFrom(input, root.GetAllocator());
	
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

