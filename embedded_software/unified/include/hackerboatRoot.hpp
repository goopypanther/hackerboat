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
 
#include <stdlib.h>
#include <inttypes.h>
#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <locale>
#include "hal/config.h"
#include "sqliteStorage.hpp"
#include "date.h"
#include "tz.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <ostream>

// forward declarations
class HackerboatStateStorage;
class SQLiteParameterSlice;
class SQLiteRowReference;

// namespaces used to make the time functions readable

using namespace date;
using namespace std::chrono;
using namespace rapidjson;
using namespace std;

// type definitions for code sanity

typedef time_point<system_clock> 	sysclock;
typedef duration<system_clock>		sysdur;

#define USE_RESULT __attribute__((warn_unused_result))

/**
 * @class HackerboatState
 *
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 */

class HackerboatState {
	public:
		/** Populate the object from the given json object.
		 * If seq is true, a sequence number element is expected
		 */
		virtual bool parse (Value& input) USE_RESULT = 0;

		/** Pack the contents of the object into a json object and return a pointer to that object.
		 * If seq is true, a sequence number element will be included
		 */
		virtual Value pack () const USE_RESULT = 0;

		/** Tests whether the current object is in a valid state */
		virtual bool isValid (void) const {
			return true;
		};

		//static json_t *HackerboatState::packTime (std::chrono::time_point<std::chrono::system_clock> t);
		//static int parseTime (json_t *input, std::chrono::time_point<std::chrono::system_clock> *t) USE_RESULT;
		
		sysclock recordTime;
		
		/**
		 * @function packTime
		 *
		 * @brief Function for packing sysclock objects into ISO 8601 date/time strings
		 * @param t The time to pack
		 * @return The time in ISO 8601, UTC, with millisecond precision
		 *
		 * This function is based on the implementation described here: 
		 * http://stackoverflow.com/questions/26895428/how-do-i-parse-an-iso-8601-date-with-optional-milliseconds-to-a-struct-tm-in-c
		 */
		 
		static std::string packTime (sysclock t) {
			std::ostringstream output;
			sys_time<milliseconds> tp = floor<milliseconds>(t);
			output << tp;
			return output.str();
		};
		/**
		 * @function parseTime
		 *
		 * @brief Function for extracting a sysclock object from an ISO8601 date/time string
		 * @param in The input string
		 * @param t A reference to a sysclock object where the incoming time will be stored
		 * @return True is parsing is successful, false otherwise
		 */
		 
		static bool parseTime (std::string in, sysclock& t) {
			std::istringstream input {in};
			date::parse(input, "%FT%TZ", t);
			if (input.fail()){
				input.clear();
				input.str(in);
				date::parse(input, "%FT%T%Ez", t);
			}
			if (input.fail()){
				input.clear();
				input.str(in);
				date::parse(input, "%F %TZ", t);
			}
			if (input.fail()){
				input.clear();
				input.str(in);
				date::parse(input, "%F %T%Ez", t);
			}
			return true;
		};

		// helper functions for getting and setting JSON values
		bool inline static GetVar(const string name, int& var, Value& d) {
			Value myvar, default_val;
			string ptr = "/" + name;
			myvar = Pointer(ptr.c_str()).GetWithDefault(d, default_val, root.GetAllocator());
			if (myvar.IsInt()) {
				var = myvar.GetInt();
			} else return false;
			return true;
		}

		bool inline static GetVar(const string name, double& var, Value& d) {
			Value myvar, default_val;
			string ptr = "/" + name;
			myvar = Pointer(ptr.c_str()).GetWithDefault(d, default_val, root.GetAllocator());
			if (myvar.IsDouble()) {
				var = myvar.GetDouble();
			} else return false;
			return true;
		}

		bool inline static GetVar(const string name, string& var, Value& d) {
			Value myvar, default_val;
			string ptr = "/" + name;
			myvar = Pointer(ptr.c_str()).GetWithDefault(d, default_val, root.GetAllocator());
			if (myvar.IsString()) {
				var = myvar.GetString();
			} else return false;
			return true;
		}

		bool inline static GetVar(const string name, Value& var, Value &d) {
			Value default_val;
			string ptr = "/" + name;
			var = Pointer(ptr.c_str()).GetWithDefault(d, default_val, root.GetAllocator());
			return true;
		}

		bool inline static GetVar(const string name, bool& var, Value &d) {
			Value myvar, default_val;
			string ptr = "/" + name;
			myvar = Pointer(ptr.c_str()).GetWithDefault(d, default_val, root.GetAllocator());
			if (myvar.IsBool()) {
				var = myvar.GetBool();
			} else return false;
			return true;
		}

		int inline static PutVar(const string name, const int& var, Value &d) {
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Set(d, var, root.GetAllocator());
			return 0;
		}

		int inline static PutVar(const string name, const double& var, Value &d) {
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Set(d, var, root.GetAllocator());
			return 0;
		}

		int inline static PutVar(const string name, const string& var, Value &d) {
			Value s;
			s.SetString(var.c_str(), var.size(), root.GetAllocator());
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Set(d, s, root.GetAllocator());
			return 0;
		}

		int inline static PutVar(const string name, const Value& var, Value &d) {
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Set(d, var, root.GetAllocator());
			return 0;
		}

		int inline static PutVar(const string name, const bool var, Value &d) {
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Set(d, var, root.GetAllocator());
			return 0;
		}

		int inline static PutVar(const string name, Value &d) {
			string ptr = "/" + name;
			Pointer(ptr.c_str()).Create(d, root.GetAllocator());
			return 0;
		}

	protected:
		HackerboatState(void) {};
		static Document root;
};

std::ostream& operator<< (std::ostream& stream, const HackerboatState& state);

/**
 * @class HackerboatStateStorable
 *
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 * This base class connects to a database of records containing instances of the object type.
 *
 */

class HackerboatStateStorable : public HackerboatState {
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
		
	protected:
		HackerboatStateStorable()			/**< Create a state object */
			: _sequenceNum(-1)
		{};

		sequence 	_sequenceNum;				/**< sequence number in the database, or -1 */

		/** Returns a sqlite storage object for this instance.
		 *
		 * Concrete classes must implement this to return a
		 * HackerboatStateStorage object representing the database,
		 * table name, and columns of the place this instance is
		 * stored. Typically this returns a single shared storage
		 * instance for all instances of a given class, but that's not
		 * required.
		 *
		 * The columns defined by the returned storage object must
		 * match whatever this class's fillRow() and readFromRow()
		 * implementations expect.
		 */
		virtual HackerboatStateStorage& storage() = 0;

		/** Write the receiver's state into a set of sqlite columns.
		 *
		 * The default implementation calls
		 * HackerboatState::pack() and expects the database to
		 * contain a single JSON column.
		 */
		virtual bool fillRow(SQLiteParameterSlice) const USE_RESULT = 0;

		/** Populate the receiver from a database row.
		 *
		 * The default implementation expects a single column
		 * containing JSON text which is deserialized and given to
		 * HackerboatState::parse().
		 */
		virtual bool readFromRow(SQLiteRowReference, sequence) USE_RESULT = 0;
};

std::ostream& operator<< (std::ostream& stream, const Document& d) {
	StringBuffer buf;
	Writer<StringBuffer> writer(buf);
	d.Accept(writer);
	stream << buf.GetString();
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const Value& v) {
	Document d;
	Pointer("/").Set(d, v, d.GetAllocator());
	stream << d;
	return stream;
}

#endif
