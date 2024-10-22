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
#include "rapidjson/rapidjson.h"
#include "hal/config.h"
#include "gps.hpp"
#include "ais.hpp"
#include "hal/inputThread.hpp"
#include "hal/gpsdInput.hpp"
#include "pstream.h"
#include "easylogging++.h"
#include "configuration.hpp"

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
	Document root;
	bool result = true;
	string buf, s;
	buf.reserve(Conf::get()->gpsBufSize());
	int i = 0;
	while (gpsdstream.in_avail()) {
		buf.push_back(gpsdstream.sbumpc());
		i++;
		if (buf.back() == '\r')	break;
		if (i > 5000) break;
	} 
	if (buf.length() > 10) {
		//cerr << "Incoming buffer is: " << buf.c_str() << endl;
		root.Parse(buf.c_str());
		if (!root.HasParseError() && root.IsObject()) {
			//cerr << "Loaded GPS JSON string." << endl;
			if (root.HasMember("class") && root["class"].IsString()) {
				s = root["class"].GetString();
				if (s == "TPV") {
					LOG(DEBUG) << "Got GPS packet";
					LOG(DEBUG) << "GPS packet contents: " << root;
					result = _lastFix.parseGpsdPacket(root);
				} else if (s == "AIS") {
					AISShip newship;
					LOG(DEBUG) << "Got AIS packet";
					LOG(DEBUG) << "AIS packet contents: " << root;
					if (newship.parseGpsdPacket(root)) {
						_aisTargets.emplace(newship.getMMSI(), newship);
						result = true;
					} 
				} else result = false;
			} else result = false;
		} else result = false;
	} else result = false;
	
	LOG_IF(root.HasParseError(), DEBUG) << "GPSd JSON loading error: " << root.GetParseError() << " offset: " 
										 << root.GetErrorOffset() << "buffer" << buf;
	//lock.unlock();
	if (result && s == "TPV") {
		_gpsAvgList.emplace_front(_lastFix);
		if (_gpsAvgList.size() < Conf::get()->gpsAvgLen()) {
			_gpsAvgList.pop_back();
		}
	}
	return result;
}

map<int, AISShip>* GPSdInput::getData() {
	return &_aisTargets;
}

map<int, AISShip> GPSdInput::getData(AISShipType shiptype) {
	map<int, AISShip> result;
	for (auto const &r : _aisTargets) {
		if (shiptype == r.second.shiptype) {
			result.emplace(r.first, r.second);
		}
	}
	return result;
}

AISShip* GPSdInput::getData(int MMSI) {
	for (auto const &r : _aisTargets) {
		if (MMSI == r.first) {
			return &(_aisTargets[MMSI]);
		}
	}
	return NULL;
}

AISShip* GPSdInput::getData(string name) {
	for (auto const &r : _aisTargets) {
		if (name == r.second.shipname) {
			return &(_aisTargets[r.first]);
		}
	}
	return NULL;
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
