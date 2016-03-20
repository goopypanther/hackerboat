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
#include "enumtable.hpp"
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
		typedef boatModeEnum Mode;
		static const enumerationNameTable<boatModeEnum> modeNames;

		boneStateClass ();

		bool insertFault (const string fault);		/**< Add the named fault to the fault string. Returns false if fault string is full */
		bool removeFault (const string fault);		/**< Remove the named fault from the fault string. Returns false if not present */
		bool hasFault (const string fault);		/**< Returns true if given fault is present */
		int faultCount (void);				/**< Returns the current number of faults */

		bool setMode (boatModeEnum s);			/**< Set state to the given value */
		bool setCommand (boatModeEnum c);		/**< Set command to the given value */
		bool setArduinoMode (arduinoModeEnum s);	/**< Set Arduino state to the given value */

		timespec 					uTime;			/**< Time the record was made */
		timespec					lastContact;		/**< Time of the last contact from the shore station */
		boatModeEnum				mode = Mode::NONE;	/**< current mode of the beaglebone */
		boatModeEnum				command = Mode::NONE;	/**< commanded mode of the beaglebone */
		arduinoModeEnum				ardMode;		/**< current mode of the Arduino */
		string						faultString;		/**< comma separated list of faults */
		gpsFixClass					gps;			/**< current GPS position */
		int32_t						waypointNext;		/**< ID of the current target waypoint */
		double						waypointStrength;	/**< Strength of the waypoint */
		double						waypointAccuracy;	/**< How close the boat gets to each waypoint before going to the next one */
		double						waypointStrengthMax;	/**< Maximum waypoint strength */
		bool						autonomous;		/**< When set true, the boat will operate autonomously */
		locationClass					launchPoint;		/**< Location from which the boat departed */


		/* Concrete implementations of stateClassStorable */
		virtual bool parse (json_t *input, bool seq);
		virtual json_t *pack (bool seq = true) const;
		virtual bool isValid (void) const;

	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
};
static inline const std::string& toString(boatModeEnum num) {
	return boneStateClass::modeNames.get(num);
}
static inline bool fromString(std::string name, boatModeEnum *value) {
	return boneStateClass::modeNames.get(name, value);
}
inline static json_t *json(boatModeEnum num) {
	return json(boneStateClass::modeNames.get(num));
}

#endif /* BONESTATESTRUCT_H */
