/******************************************************************************
 * Hackerboat orientation input module
 * hal/gpsInput.hpp
 * This module reads gps data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <map>
#include <iostream>
#include "jansson.h"
#include "hal/config.h"
#include "gps.hpp"
#include "ais.hpp"
#include "hal/inputThread.hpp"
#include "hal/gpsdInput.hpp"
#include "pstream.h"
#include "easylogging++.h"

using namespace std;
using namespace redi;

GPSdInput::GPSdInput (string host, int port) :
	_host(host), _port(port) {
		LOG(DEBUG) << "Creating GPSdInput object with specified target " << _host << ":" << _port;
		period = 100ms;
	};
	
GPSdInput::GPSdInput () :
	_host("127.0.0.1"), _port(3001) {
		LOG(DEBUG) << "Creating GPSdInput object with default target " << _host << ":" << _port;
		period = 100ms;
	};

bool GPSdInput::setHost (string host) {
	if (host != "") {
		_host = host;
		LOG(DEBUG) << "Setting new host " << _host;
		return true;
	}
	return false;
}

bool GPSdInput::setPort (int port) {
	if ((port > 0) && (port < 65535)) {
		_port = port;
		LOG(DEBUG) << "Setting new port " << _port;
		return true;
	}
	return false;
}

bool GPSdInput::connect () {
	if ((_port > 0) && (_port < 65535) && (_host != "")) {
		string cmd = "/usr/bin/gpspipe -w " + _host + ":" + to_string(_port);
		gpsdstream.open(cmd, pstreams::pstdout | pstreams::pstderr);
		LOG(DEBUG) << "Connecting to gpsd with command " << cmd;
		std::this_thread::sleep_for(1ms);
		LOG_IF(!isConnected(), ERROR) << "Failed to connect to gpsd with command " << cmd;
		return (isConnected());
	}
	return false;
}

bool GPSdInput::disconnect () {
	gpsdstream.close();
	return true;
}

bool GPSdInput::isConnected () {
	if (/*gpsdstream.is_open() && */(!gpsdstream.exited())) return true;
	return false;
}

bool GPSdInput::begin() {
	if (this->connect()) {
		this->myThread = new std::thread (InputThread::InputThreadRunner(this));
		myThread->detach();
		LOG(INFO) << "GPS subsystem started";
		return true;
	}
	LOG(FATAL) << "Unable to initialize GPS subsystem";
	return false;
}

bool GPSdInput::execute() {
	// grab the lock
	//if (!lock && (!lock.try_lock_for(IMU_LOCK_TIMEOUT))) return false;
	bool result = true;
	json_error_t inerr;
	string buf;
	buf.reserve(GPS_BUF_SIZE);
	int i = 0;
	while (gpsdstream.in_avail()) {
		buf.push_back(gpsdstream.sbumpc());
		i++;
		if (buf.back() == '\r')	break;
		if (i > 5000) break;
	} 
	if (buf.length() > 10) {
		cout << "Incoming buffer is: " << buf.c_str() << endl;
		json_t* input = json_loads(buf.c_str(), JSON_REJECT_DUPLICATES, &inerr);
		if (input) {
			cout << "Loaded GPS JSON string." << endl;
			//buf.clear();
			json_t* objclass = json_object_get(input, "class");
			if (objclass) {
				string s;
				const char *p = json_string_value(objclass);
				size_t l = json_string_length(objclass);
				s.assign(p, l);
				if (s == "TPV") {
					LOG(DEBUG) << "Got GPS packet";
					LOG(DEBUG) << "GPS packet contents: " << input;
					result = _lastFix.parseGpsdPacket(input);
				} else if (s == "AIS") {
					AISShip newship;
					LOG(DEBUG) << "Got AIS packet";
					LOG(DEBUG) << "AIS packet contents: " << input;
					if (newship.parseGpsdPacket(input)) {
						_aisTargets.emplace(newship.getMMSI(), newship);
						result = true;
					} 
				} 
				//json_decref(input);
				//json_decref(objclass);
			} 
			json_decref(input);
			result = false;
		} result = false;
	} else result = false;
	
	LOG_IF(strlen(inerr.text), DEBUG) << "GPSd JSON loading error: " << inerr.text << " from " << inerr.source 
									<< " at line " << inerr.line << ", column " << inerr.column << ", buffer: " << buf;
	//lock.unlock();
	if (result) {
		_gpsAvgList.emplace_front(_lastFix);
		if (_gpsAvgList.size() < GPS_AVG_LEN) {
			_gpsAvgList.pop_back();
		}
	}
	return result;
}

map<int, AISShip> GPSdInput::getData() {
	return _aisTargets;
}

map<int, AISShip> GPSdInput::getData(AISShipType type) {
	map<int, AISShip> result;
	for (auto const &r : _aisTargets) {
		if (type == r.second.shiptype) {
			result.emplace(r.first, r.second);
		}
	}
	return result;
}

AISShip GPSdInput::getData(int MMSI) {
	return _aisTargets[MMSI];
}

AISShip GPSdInput::getData(string name) {
	for (auto const &r : _aisTargets) {
		if (name == r.second.shipname) {
			return r.second;
		}
	}
	return AISShip();
}

int GPSdInput::pruneAIS(Location loc) {
	int count = 0;
	for (auto &r : _aisTargets) {
		if (r.second.prune(loc)) {
			LOG(DEBUG) << "Pruning AIS target " << r.second.pack();
			_aisTargets.erase(r.first);
			count++;
		}
	}
	return count;
}

GPSFix GPSdInput::getAverageFix() {
	double lattot = 0, lontot = 0;
	GPSFix result;
	for (auto &thisfix: _gpsAvgList) {
		lattot += thisfix.fix.lat;
		lontot += thisfix.fix.lon;
	}
	result.fix.lat = lattot/_gpsAvgList.size();
	result.fix.lon = lontot/_gpsAvgList.size();
	return result;
	
}
