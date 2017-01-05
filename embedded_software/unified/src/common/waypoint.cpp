/******************************************************************************
 * Hackerboat waypoint module
 * waypoint.cpp
 * This module parses the kml file that defines the waypoints and presents them.
 * Much code copied shamelessly from the libkml examples, particularly 
 * printgeometry.cc
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "location.hpp"
#include "hackerboatRoot.hpp"
#include "hal/config.h"
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "waypoint.hpp"
#include "easylogging++.h"

using namespace std;

Waypoints::Waypoints () : kmlPath(""), action(WaypointActionEnum::NONE),
	waypoints({}), _c(0) {};

Waypoints::Waypoints (std::string kmlFile) : kmlPath(kmlFile), 
	action(WaypointActionEnum::NONE), waypoints({}), _c(0) {};

bool Waypoints::fetchKML (std::string url) {
	if (access(kmlPath.c_str(), W_OK)) return false;	// check that we can write to the file we're trying to retrieve to
	std::string cmd = "/usr/bin/curl -f -o ";			// call cURL with error messages suppressed and output to a given file
	cmd += kmlPath;
	cmd += " " + url;
	LOG(INFO) << "Fetching KML file with following command: " << cmd;
	if (system(cmd.c_str())) return false;				// call curl and return false if it fails
	return true;
}

bool Waypoints::loadKML () {
	std::ifstream in;
	std::string line;
	std::string coordinates;
	bool gotPlacemark = false;
	bool gotName = false;
	bool gotLineString = false;
	bool result = false;
	int linecount = 0;
	LOG(INFO) << "Loading KML file " << kmlPath;
	in.open(kmlPath);
	if (in.is_open()) {																				// if we successfully opened the file, proceed 
		while (in.good() && !gotLineString) {														// search through the file while it's still good and we haven't found a LineString object
			std::getline(in, line);																	// suck in the next line
			linecount++;
			if (!gotPlacemark) {																	// if we have not yet found a <Placemark>, see if this line contains one
				if (line.find("<Placemark>") != std::string::npos) {
					gotPlacemark = true;
					VLOG(3) << "Found start of Placemark on line:" << to_string(linecount) << "[" << line << "]";
				}
			} else {
				if (line.find("</Placemark>") != std::string::npos) {								// if we've found a <Placemark> but it closes without hitting anything else, go back to the original state  
					VLOG(3) << "Found close of Placemark with no name on line:" << to_string(linecount) << "[" << line << "]";
					gotPlacemark = false;
					gotName = false;
				}
			}
			if (gotPlacemark && !gotName) {															// if we have found a <Placemark> but no <name>, see if this line contains one
				size_t start = line.find("<name>");
				if (start != std::string::npos) {
					VLOG(3) << "Found name on line:" << to_string(linecount) << "[" << line << "]";
					gotName = true;
					size_t stop = line.find("Waypoints</name>", start);							// if it contains a <name> tag, see if it has the right name (Waypoints)
					if (stop == std::string::npos) {												// if not, skip over this <Placemark>
						gotName = false;
						gotPlacemark = false;
					} else {
					}
				}
			} 
			if (gotPlacemark && gotName && !gotLineString) {										// if we have found a <Placemark> with the right <name>, look for a <LineString> object
				if (line.find("<LineString>") != std::string::npos) {
					VLOG(3) << "Found start of line string on line: " << to_string(linecount) << "[" << line << "]";
					gotLineString = true;
				}
			}
		}
		if (gotLineString) {																		// if we found a <LineString> object, proceed
			bool gotCoordinates = false;
			bool finishedCoordinates = false;
			bool newlineflag = true;
			while (in.good() && !finishedCoordinates) {												// Search through the file until we reach the end or we find a valid coordinate list
				std::getline(in, line);															// This is to prevent doubling up
				if (!gotCoordinates) {																// If we haven't found the opening <coordinates> tag, look for it
					size_t start = line.find("<coordinates>");
					if (start != std::string::npos) {
						gotCoordinates = true;
						coordinates = line.substr(line.find(">", start) + 1);						// chop off the <coordinates> tag to ease later parsing.
						newlineflag = false;
					}
				}
				if (gotCoordinates) {																// If we've already found the opening <coordinates> tag, look for the closing tag
					if (newlineflag) coordinates += " " + line + " ";								// Add the newly loaded line to our coordinates string, unless this is not a new line. Spaces are added for the benefit of string::stod()
					if (line.find("</coordinates>") != std::string::npos) finishedCoordinates = true;	// And if we've got the closing tag, set the exit condition
				}
				newlineflag = true;
			}
			if (finishedCoordinates) {
				size_t endofnum;					// position of the end of the number we're currently converting
				double tmplat, tmplon;				// temporary latitude & longitude
				bool done = false;					// loop state flag 
				this->waypoints.clear();			// clear the waypoints list
				while (!done) {
					try {
						tmplon = std::stod(coordinates, &endofnum);				// grab the longitude
						coordinates.erase(0, endofnum + 1);							// and erase it
						tmplat = std::stod(coordinates, &endofnum);				// grab the latitude
						coordinates.erase(0, endofnum + 1);							// and erase it
						std::stod(coordinates, &endofnum);						// discard the altitude
						coordinates.erase(0, endofnum);							// and erase it
						waypoints.emplace_back(Location(tmplat, tmplon));		// Add the new waypoint to the list
						result = true;											// Returns true as long as we get at least one good point
						VLOG(2) << "Got waypoint: " << waypoints.back();
					} catch (...) {												// if we throw an exception here, it's likely because we are done
						done = true;
					}
				}
				
			}
		}
	} 
	in.close();
	return result;
}

bool Waypoints::loadKML (std::string kmlFile) {
	if (access(kmlFile.c_str(), R_OK)) { 	// Make sure the file exists and is readable; otherwise, exit
		LOG(ERROR) << "Can't read file: " << kmlFile << endl;
		return false; 
	}
	kmlPath = kmlFile;
	return this->loadKML();
}

bool Waypoints::setKMLPath (std::string kmlFile) {
	kmlPath = kmlFile;
	VLOG(2) << "New KML file is: " << kmlPath;
	return true;
}

Location Waypoints::getWaypoint (unsigned int waypoint) {
	if (waypoints.size() == 0) {
		Location wp;
		return wp;
	}
	if (waypoint >= waypoints.size()) return waypoints.back();
	return waypoints.at(waypoint);
}

Location Waypoints::getWaypoint () {
	if (waypoints.size() == 0) {
		Location wp;
		return wp;
	}
	if (_c >= waypoints.size()) _c = (waypoints.size() - 1);
	return getWaypoint(_c);
}

void Waypoints::setCurrent (unsigned int waypoint) {
	if (waypoint >= waypoints.size()) waypoint = (waypoints.size() - 1);
	LOG(INFO) << "Setting current waypoint to " << to_string(waypoint);
	_c = waypoint;
}

bool Waypoints::increment () {
	if (action == WaypointActionEnum::IDLE) return false;
	if ((_c + 1) == waypoints.size()) {
		if (action == WaypointActionEnum::REPEAT) {
			LOG(INFO) << "Repeating waypoints...";
			_c = 0;
			return true;
		} else {
			LOG(INFO) << "Reached end of waypoint list, not repeating.";
			return false;
		}
	}
	_c++;
	LOG(INFO) << "Setting current waypoint to " << to_string(_c);
	return true;
}

bool Waypoints::decrement () {
	if (action == WaypointActionEnum::IDLE) return false;
	if (_c == 0) return false;
	_c--;
	LOG(INFO) << "Setting current waypoint to " << to_string(_c);
	return true;
}

const EnumNameTable<WaypointActionEnum> Waypoints::actionNames = {
	"Idle",
	"Anchor",
	"Return",
	"Repeat",
	"None"
};
