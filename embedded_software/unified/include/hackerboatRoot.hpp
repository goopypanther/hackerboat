/**************************************************************************//**
 * @brief Hackerboat root class
 * @file  hackerboatRoot.hpp
 *
 * This modules is compiled into the other modules to give a common interface
 * for json and database storage.
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Aug 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef HACKERBOATROOT_H
#define HACKERBOATROOT_H
 
#include <jansson.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <string>
#include <memory>
#include <vector>
#include "config.h"
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

#endif