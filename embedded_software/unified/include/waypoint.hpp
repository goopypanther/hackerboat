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
#include "logs.hpp"
#include "location.hpp"
#include "hackerboatRoot.hpp"
#include "hal/config.h"
#include "enumdefs.hpp"
#include "enumtable.hpp"

class Waypoints {
	public:
		static const EnumNameTable<WaypointActionEnum> actionNames;
	
		Waypoints ();								/**< Create an empty Waypoints object */
		Waypoints (std::string kmlFile);			/**< Create a Waypoints object from the given KML file */
		bool loadKML ();							/**< (Re)load the waypoints from the stored KML path. Returns false if no file or file cannot be loaded. */
		bool loadKML (std::string kmlFile);			/**< Changes the stored KML path to kmlFile and loads it. Returns false if it cannot be loaded. */
		bool fetchKML (std::string url);			/**< Fetch KML file from remote source and store it in the location pointed at by kmlPath. */
		bool setKMLPath (std::string kmlFile);		/**< Set the KML file location but do not load it */
		std::string getKMLPath () {return kmlPath;};
		Location getWaypoint (unsigned int waypoint);	/**< Retrieve the given waypoint. */
		Location getWaypoint ();					/**< Retrieve the current waypoint */
		void setCurrent (unsigned int waypoint);		/**< Set the current waypoint */
		bool increment ();							/**< Increment the current waypoint. Behavior is affected by the action variable -- this returns false if there is no new waypoint and the action is anything other than REPEAT. */
		bool decrement ();							/**< Decrement the current waypoint. Returns false if already on the first waypoint, true otherwise. */
		int count () {return waypoints.size();};	/**< Returns the number of waypoints */
		int current () {return _c;};				/**< Returns the current waypoint number */
		WaypointActionEnum getAction () {return action;};			/**< Returns the action to take at the end of the waypoint list */
		void setAction (WaypointActionEnum act) {action = act;};	/**< Set the action to take at the end of the waypoint list. This effects the response to increment() and is used by the appropriate mode to decide the next action. */
		
	private:
		std::string 		kmlPath;		/**< The full path of the KML file to use. */
		WaypointActionEnum 	action;			/**< The action to take at the end of the waypoint list. */
		vector<Location>	waypoints;		/**< A vector of all of the waypoints. */
		unsigned int		_c;				/**< The index of the current waypoint. */
};

#endif /* WAYPOINT_H */
