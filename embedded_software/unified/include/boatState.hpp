/******************************************************************************
 * Hackerboat Beaglebone boat state module
 * boatState.hpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef BOATSTATE_H
#define BOATSTATE_H

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "location.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"
#include "hal/config.h"
#include "logs.hpp"
#include "enumdefs.hpp"
#include "healthMonitor.hpp"
//#include "waypoints.hpp"
#include "dodge.hpp"

using namespace std;

class boatStateClass : public hackerboatStateClassStorable {
	public:
		static const enumerationNameTable<boatModeEnum> boatModeNames;
		static const enumerationNameTable<navigationModeEnum> navModeNames;
		static const enumerationNameTable<autoModeEnum> autoModeNames;
		static const enumerationNameTable<rcModeEnum> rcModeNames;
		
		boatStateClass ();
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid ();
		hackerboatStateStorage& storage();
		
		bool insertFault (const std::string fault);			/**< Add the named fault to the fault string. Returns false if fault string is full */
		bool removeFault (const std::string fault);			/**< Remove the named fault from the fault string. Returns false if not present */
		bool hasFault (const std::string fault) const;		/**< Returns true if given fault is present */
		int faultCount (void) const;						/**< Returns the current number of faults */
 		void clearFaults () {faultString = "";};			/**< Remove all faults */
		std::string getFaultString() {return faultString;};	/**< Returns the entire fault string */
		bool setBoatMode (boatModeEnum m) {_boat = m; return true;};		/**< Set boat mode to the given value */
		bool setBoatMode (std::string mode);				/**< Set boat mode to the given value */
		boatModeEnum getBoatMode () {return _boat;};		/**< Return boat mode */
		bool setNavMode (navigationModeEnum m) {_nav = m; return true;};		/**< Set nav mode to the given value */
		bool setNavMode (std::string mode);					/**< Set nav mode to the given value */
		navigationModeEnum getNavMode () {return _nav;};
		bool setAutoMode (autoModeEnum m) {_auto = m; return true;};		/**< Set autonomous mode to the given value */
		bool setAutoMode (std::string mode);				/**< Set autonomous mode to the given value */
		autoModeEnum getAutoMode () {return _auto;};
		bool setRCmode (rcModeEnum m) {_rc = m; return true;};			/**< Set RC mode to the given value */
		bool setRCmode (std::string mode);					/**< Set RC mode to the given value */
		rcModeEnum getRCMode () {return _rc;};
			
		int 						currentWaypoint; 	/**< The current waypoint */
		double						waypointStrength;	/**< Relative strength of the waypoint */
		sysclock					lastContact;	/**< Time of last shore contact */
		sysclock					lastRC;			/**< Time of the last signal from the RC input */
		locationClass				lastFix;		/**< Location of the last GPS fix */
		locationClass				launchPoint;	/**< Location of the launch point */
//		waypointListClass			waypoints;		/**< Waypoints to follow */
		waypointActionEnum			action;			/**< Action to take at the last waypoint */
		dodgeClass					diversion;		/**< Avoid obstacles! */
		healthMonitorClass			health;			/**< Current state of the boat's health */
		tuple<double, double, double> K;			/**< Steering PID gains. Proportional, integral, and differential, respectively. */ 
		
	private:
		std::string 	faultString;
		boatModeEnum 	_boat;
		navigationModeEnum		_nav;
		autoModeEnum 	_auto;
		rcModeEnum 		_rc;
	
};
#endif 
