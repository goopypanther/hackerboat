/******************************************************************************
 * Hackerboat Beaglebone types module
 * stateStructTypes.hpp
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
 * see the Hackerboat documentation for more details
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
#include "gps.hpp"

using namespace string;

// buffer & string sizes
#define STATE_STRING_LEN		30
#define GPS_SENTENCE_LEN		120
#define	FAULT_STRING_LEN		1024
#define NAV_SOURCE_NAME_LEN		30
#define NAV_VEC_LIST_LEN		30

// value limits 

/**
 * @class hackerboatStateClass 
 * 
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 */

class hackerboatStateClass {
	public:
		virtual bool parse (json_t *input, bool seq = true);/**< Populate the object from the given json object. If seq is true, a sequence number element is expected */
		virtual json_t *pack (bool seq = true);				/**< Pack the contents of the object into a json object and return a pointer to that object. If seq is true, a sequence number element will be included */
		virtual bool isValid (void) const {return true;};	/**< Tests whether the current object is in a valid state */
		
	protected:	
		hackerboatStateClass(void) = default;
		json_t *packTimeSpec (timespec t);
		int parseTimeSpec (json_t *input, timespec *t);
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
		hackerboatStateClassStorable(void);
		hackerboatStateClassStorable(const string file);		/**< Create a state object attached to the given file */
		int32_t getSequenceNum (void) {return _sequenceNum;};	/**< Get the sequenceNum of this object (-1 until populated from a file) */
		bool openFile(const string name);						/**< Open the given database file & store the name */
		bool openFile(void);									/**< Open the stored database file */
		bool closeFile(void);									/**< Close the open file */
		int32_t count (void);									/**< Return the number of records of the object's type in the open database file */
		bool writeRecord (void);								/**< Write the current record to the target database file */
		bool getRecord(int32_t select);							/**< Populate the object from the open database file */
		bool getLastRecord(void);								/**< Get the latest record */
		virtual bool insert(int32_t num) {return false;};		/**< Insert the contents of the object into the database table at the given point */
		bool append(void);										/**< Append the contents of the object to the end of the database table */
		
	protected:
		int32_t 	_sequenceNum = -1;	/**< sequence number */
		string 		_fileName;			/**< database filename (with path) */
		sqlite3 	*_db;				/**< database handle */
};

/**
 * @class orientationClass
 *
 * @brief An orientation, received from the Arduino 
 *
 */

class orientationClass : public hackerboatStateClass {
	public:
		orientationClass(double r, double p, double y):
							pitch(p), roll(r), yaw(y);
		bool normalize (void);
		double roll 	= NAN;
		double pitch 	= NAN;
		double heading 	= NAN;
	private:
		static const string _format = "{s:f,s:f,s:f}";
		static const double	maxVal = 180.0;
		static const double	minVal = -180.0;
};
		
/**
 * @class waypointClass
 *
 * @brief This is the class for storing & manipulating waypoints
 *
 */
 
class waypointClass : public hackerboatStateClassStorable {
	public:	
		enum actionEnum {
			STOP = 0,						
			HOME = 1,
			CONTINUE = 2,
		};
		typedef int16_t indexT;
		waypointClass (void);
		waypointClass (locationClass loc, actionEnum action = CONTINUE); 	/**< Create a waypoint at loc with action */
		
		bool			setAction(actionEnum action);			/**< Set the action to take when this waypoint is reached */
		actionEnum		getAction(void);						/**< Return the action that this waypoint is set to */

		waypointClass 	*getNextWaypoint(void);					/**< return the next waypoint to travel towards */
		bool			setNextWaypoint(waypointClass* next);		/**< Set the next waypoint to the given object (works only if it has a sequenceNum > 0; renumber indices as necessary */
		bool			setNextWaypoint(indexT index);			/**< As above, but set by current index; renumbering proceeds as above */
		indexT			getNextIndex(void);				/**< Return the index of the next waypoint */

		/* Concrete implementations of stateClassStorable */
		bool parse (json_t *, bool);
		json_t *pack (bool) const;
		bool isValid (void) const;

		locationClass location;
	private:
		indexT			index = -1;					/**< Place of this waypoint in the waypoint list */ 
		sequence		nextWaypoint = -1;				/**< _sequenceNum of the next waypoint */
		actionEnum		act;						/**< Action to perform when reaching a location */	
		static const int8_t minActionEnum = 0;
		static const int8_t maxActionEnum = 3;

	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
		virtual bool fillRow(sqliteParameterSlice) const;
		virtual bool readFromRow(sqliteRowReference, sequence);
};
static const char *string(waypointClass::actionEnum);


#endif /* STATESTRUCTTYPES_H */
