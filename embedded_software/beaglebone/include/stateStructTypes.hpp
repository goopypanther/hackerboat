/**************************************************************************//**
 * @brief Hackerboat Beaglebone types module
 * @file  stateStructTypes.hpp
 *
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Jan 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef STATESTRUCTTYPES_H
#define STATESTRUCTTYPES_H
 
#include <jansson.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <string>
#include <memory>
#include <vector>
#include "config.h"
#include "location.hpp"
#include "json_utilities.hpp"
#include "sqliteStorage.hpp"

// forward declarations
class hackerboatStateStorage;
class sqliteParameterSlice;
class sqliteRowReference;

#define USE_RESULT __attribute__((warn_unused_result))

/**
 * @class hackerboatStateClass
 *
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 */

class hackerboatStateClass {
	public:
		/** Populate the object from the given json object.
		 * If seq is true, a sequence number element is expected
		 */
		virtual bool parse (json_t *input, bool seq) USE_RESULT = 0;

		/** Pack the contents of the object into a json object and return a pointer to that object.
		 * If seq is true, a sequence number element will be included
		 */
		virtual json_t *pack (bool seq = true) const USE_RESULT = 0;

		/** Tests whether the current object is in a valid state */
		virtual bool isValid (void) const {return true;};

		static json_t *packTimeSpec (timespec t);
		static int parseTimeSpec (json_t *input, timespec *t) USE_RESULT;

	protected:
		hackerboatStateClass(void) {};
};

inline json_t *json(timespec t) {
	return hackerboatStateClass::packTimeSpec(t);
}
inline bool parse(json_t *input, timespec *t) USE_RESULT;
inline bool parse(json_t *input, timespec *t) {
	return hackerboatStateClass::parseTimeSpec(input, t) == 0;
};


/**
 * @class hackerboatStateClassStorable
 *
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 * This base class connects to a database of records containing instances of the object type.
 *
 */

class hackerboatStateClassStorable : public hackerboatStateClass {
	public:
		typedef int64_t sequence;			/**< The type of sequence numbers / OIDs in persistent storage. Negative numbers indicate invalid / missing data */

		/** Get the sequenceNum of this object.
		 * An object's sequence number is -1 until populated from or inserted into a file.
		 */
		sequence getSequenceNum (void) const
		{ return _sequenceNum; }

		sequence countRecords (void);							/**< Return the number of records of the object's type in the open database file */
		bool writeRecord (void);								/**< Update the current record in the target database file. Must already exist */
		bool getRecord(sequence select) USE_RESULT;				/**< Populate the object from the open database file */
		bool getLastRecord(void) USE_RESULT;					/**< Get the latest record */
		bool appendRecord(void);								/**< Append the contents of the object to the end of the database table. Updates the receiver's sequence number field with its newly-assigned value */
		
		/** Releases the storage object for the instance
		 * 
		 * Release all locks on the current object's storage and (optionally) close it.
		 * This must be implemented by concrete classes that implement this class. 
		 *
		 */
		 
		virtual void release(void) = 0;
		
	protected:
		hackerboatStateClassStorable()			/**< Create a state object */
			: _sequenceNum(-1)
		{};

		sequence 	_sequenceNum;				/**< sequence number in the database, or -1 */

		/** Returns a sqlite storage object for this instance.
		 *
		 * Concrete classes must implement this to return a
		 * hackerboatStateStorage object representing the database,
		 * table name, and columns of the place this instance is
		 * stored. Typically this returns a single shared storage
		 * instance for all instances of a given class, but that's not
		 * required.
		 *
		 * The columns defined by the returned storage object must
		 * match whatever this class's fillRow() and readFromRow()
		 * implementations expect.
		 */
		virtual hackerboatStateStorage& storage() = 0;

		/** Write the receiver's state into a set of sqlite columns.
		 *
		 * The default implementation calls
		 * hackerboatStateClass::pack() and expects the database to
		 * contain a single JSON column.
		 */
		virtual bool fillRow(sqliteParameterSlice) const USE_RESULT;

		/** Populate the receiver from a database row.
		 *
		 * The default implementation expects a single column
		 * containing JSON text which is deserialized and given to
		 * hackerboatStateClass::parse().
		 */
		virtual bool readFromRow(sqliteRowReference, sequence) USE_RESULT;
};

/**
 * @class orientationClass
 *
 * @brief An orientation, received from the Arduino 
 *
 */

class orientationClass {
	public:
		orientationClass() {};
		orientationClass(double r, double p, double y)
		  : pitch(p), roll(r), heading(y)
		{};
		bool normalize (void);
		double roll 	= NAN;
		double pitch 	= NAN;
		double heading 	= NAN;

		bool parse (json_t *) USE_RESULT;
		json_t *pack (void) const;
		bool isValid (void) const;

	private:
		static const double constexpr	maxVal = 180.0;
		static const double constexpr	minVal = -180.0;
};
		
/**
 * @class waypointClass
 *
 * @brief This is the class for storing & manipulating waypoints
 *
 */
 
class waypointClass : public hackerboatStateClassStorable {
	public:	
		enum class action {
			STOP = 0,						
			HOME = 1,
			CONTINUE = 2,
		};

		typedef int16_t indexT;
		waypointClass (void);
		waypointClass (locationClass loc, action action = action::CONTINUE); 	/**< Create a waypoint at loc with action */
		
		void			setAction(action act) {this->_act = act;};				/**< Set the action to take when this waypoint is reached */
		action			getAction(void) {return this->_act;};					/**< Return the action that this waypoint is set to */

		waypointClass 	*getNextWaypoint(void);									/**< return the next waypoint to travel towards */
		bool			setNextWaypoint(waypointClass* next);					/**< Set the next waypoint to the given object (works only if it has a sequenceNum > 0; renumber indices as necessary */
		bool			setNextWaypoint(indexT index);							/**< As above, but set by current index; renumbering proceeds as above */
		indexT			getNextIndex(void);										/**< Return the index of the next waypoint */

		void release(void) {
			waypointStorage->closeDatabase();
			delete waypointStorage;
		}
		
		/* Concrete implementations of stateClassStorable */
		bool parse (json_t *, bool) USE_RESULT;
		json_t *pack (bool seq = true) const;
		bool isValid (void) const;

		locationClass location;
		
		~waypointClass (void) {release();}
		
	private:
		indexT			index = -1;					/**< Place of this waypoint in the waypoint list */ 
		sequence		nextWaypoint = -1;			/**< _sequenceNum of the next waypoint */
		action			_act;						/**< Action to perform when reaching a location */	
		static const int8_t minActionEnum = 0;
		static const int8_t maxActionEnum = 3;
		hackerboatStateStorage *waypointStorage;

		
	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
		virtual bool fillRow(sqliteParameterSlice) const;
		virtual bool readFromRow(sqliteRowReference, sequence);
};
const char *toString(waypointClass::action);
bool fromString(const char *, waypointClass::action *) USE_RESULT;

/**
 * @enum arduinoModeEnum
 * @brief An enum to store the current operating mode of the Arduino.
 */
enum class arduinoModeEnum {
	POWERUP     = 0,  		/**< The boat enters this state at the end of initialization */
	ARMED		= 1,  		/**< In this state, the boat is ready to receive go commands over RF */
	SELFTEST   	= 2,  		/**< After powerup, the boat enters this state to determine whether it's fit to run */
	DISARMED   	= 3,  		/**< This is the default safe state. No external command can start the motor */
	ACTIVE     	= 4,  		/**< This is the normal steering state */
	LOWBATTERY  = 5,  		/**< The battery voltage has fallen below that required to operate the motor */
	FAULT    	= 6,  		/**< The boat is faulted in some fashion */
	SELFRECOVERY= 7,   		/**< The Beaglebone has failed and/or is not transmitting, so time to self-recover*/
	ARMEDTEST	= 8,		/**< The Arduino is accepting specific pin read/write requests for hardware testing. */
	ACTIVERUDDER= 9,		/**< The Arduino is accepting direct rudder commands */
	NONE		= 10		/**< Provides a null value for no command yet received */
};

/**
 * @brief Beaglebone controller mode, representing the overall system mode of the boat
 */
enum class boatModeEnum {
	START		= 0,  		/**< Initial starting state         */
	SELFTEST	= 1,  		/**< Initial self-test            */
	DISARMED	= 2,  		/**< Disarmed wait state          */  
	FAULT		= 3,		/**< Beaglebone faulted           */ 
	ARMED		= 4,		/**< Beaglebone armed & ready to navigate */ 
	MANUAL		= 5,		/**< Beaglebone manual steering       */ 
	WAYPOINT	= 6,		/**< Beaglebone navigating by waypoints   */
	NOSIGNAL	= 7,		/**< Beaglebone has lost shore signal    */
	RETURN		= 8,		/**< Beaglebone is attempting to return to start point */
	ARMEDTEST	= 9,		/**< Beaglebone accepts all commands that would be valid in any unsafe state */
	NONE		= 10		/**< State of the Beaglebone is currently unknown	*/
};

#undef USE_RESULT

#endif /* STATESTRUCTTYPES_H */
