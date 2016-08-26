/******************************************************************************
 * Hackerboat waypoint module
 * waypoint.hpp
 * This module parses the kml file that defines the waypoints and presents them.
 * Much code copied shamelessly from the libkml examples, particularly 
 * printgeometry.cc
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <string>
#include "kml/dom.h"
#include "kml/base/file.h"
#include "logs.hpp"
#include "location.hpp"
#include "hackerboatRoot.hpp"
#include "hal/config.h"

class Waypoints {
	public:
		Waypoints ();
		Waypoints (std::string kmlFile);
		bool loadKML ();
		bool loadKML (std::string kmlFile);
		bool fetchKML (std::string url);
		Location getWaypoint (int waypoint);
		Location getWaypoint ();
		void setCurrent (int waypoint);
		bool increment ();
		bool decrement ();
		int count ();
		int current ();
		WaypointActionEnum getAction ();
		void setAction (WaypointActionEnum act);
		
	private:
		std::string 		kmlPath;
		WaypointActionEnum 	action;
		vector<Location>	waypoints;
		int					_c;
};

#endif /* WAYPOINT_H */
