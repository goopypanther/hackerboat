/******************************************************************************
 * Hackerboat Beaglebone types module
 * boneState.hpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef BONESTATESTRUCT_H
#define BONESTATESTRUCT_H

#include <time.h>
#include <string>
#include "stateStructTypes.hpp" 
#include "arduinoState.hpp"
#include "location.hpp"
#include "gps.hpp"


/**
 * @class boneStateClass
 *
 * @brief Class for storing the current state of the Beaglebone element
 *
 */

class boneStateClass : public hackerboatStateClassStorable {
	public:
		/**
		 * @brief Beaglebone state
		 */
		enum boneStateEnum {
			BONE_START			= 0,  		/**< Initial starting state         */
			BONE_SELFTEST		= 1,  		/**< Initial self-test            */
			BONE_DISARMED		= 2,  		/**< Disarmed wait state          */  
			BONE_FAULT			= 3,		/**< Beaglebone faulted           */ 
			BONE_ARMED			= 4,		/**< Beaglebone armed & ready to navigate */ 
			BONE_MANUAL			= 5,		/**< Beaglebone manual steering       */ 
			BONE_WAYPOINT		= 6,		/**< Beaglebone navigating by waypoints   */
			BONE_NOSIGNAL		= 7,		/**< Beaglebone has lost shore signal    */
			BONE_RETURN			= 8,		/**< Beaglebone is attempting to return to start point */
			BONE_ARMEDTEST		= 9,		/**< Beaglebone accepts all commands that would be valid in any unsafe state */
			BONE_NONE			= 10		/**< State of the Beaglebone is currently unknown	*/
		};
	
		bool insertFault (const string fault);	/**< Add the named fault to the fault string. Returns false if fault string is full */
		bool removeFault (const string fault);	/**< Remove the named fault to the fault string. Returns false if not present */
		bool hasFault (const string fault);		/**< Returns true if given fault is present */
		int faultCount (void);					/**< Returns the current number of faults */
		bool setState (boneStateEnum s);		/**< Set state to the given value */
		bool setCommand (boneStateEnum c);		/**< Set command to the given value */
		bool setArduinoState (arduinoStateClass::arduinoStateEnum s); /**< Set Arduino state to the given value */
		
		timespec 					uTime;				/**< Time the record was made */
		timespec					lastContact;		/**< Time of the last contact from the shore station */
		boneStateEnum				state = BONE_NONE;	/**< current state of the beaglebone */	
		string						stateString;		/**< current state of the beaglebone, human readable string */
		boneStateEnum				command = BONE_NONE;/**< commanded state of the beaglebone */
		string						commandString;		/**< commanded state of the beaglebone, human readable string */
		arduinoStateClass::arduinoStateEnum			ardState;		/**< current state of the Arduino */
		string						ardStateString;		/**< current state of the Arduino, human readable string */
		string						faultString;		/**< comma separated list of faults */
		gpsFixClass					gps;				/**< current GPS position */
		int32_t						waypointNext;		/**< ID of the current target waypoint */
		double						waypointStrength;	/**< Strength of the waypoint */
		double						waypointAccuracy;	/**< How close the boat gets to each waypoint before going to the next one */
		double						waypointStrengthMax;/**< Maximum waypoint strength */
		bool						autonomous;			/**< When set true, the boat will operate autonomously */	
		locationClass				launchPoint;		/**< Location from which the boat departed */
		
		static const string boneStates[] = {
			"Start", 
			"SelfTest", 
			"Disarmed", 
			"Fault",
			"Armed", 
			"Manual", 
			"WaypointNavigation",
			"LossOfSignal", 
			"ReturnToLaunch", 
			"ArmedTest",
			"None"
		};		
		static const uint8_t boneStateCount = 11;
		
	private:
		void initHashes (void);								/**< Initialize state name hashes */
		static const char *_format = "{s:o,s:o,s:i,s:s,s:i,s:s,s:i,s:s,s:s,s:o,s:i,s:f,s:f,s:f,s:b,s:o}";
		static uint32_t stateHashes[boneStateCount];		/**< All the state names, hashed for easy lookup */
		

};

#endif /* BONESTATESTRUCT_H */
