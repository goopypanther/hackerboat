/******************************************************************************
 * Hackerboat Beaglebone storable state objects
 * Kind of a simple ORM
 *
 * Version 0.1
 *
 ******************************************************************************/

#include "sqliteStorage.hpp"
#include "hackerboatRoot.hpp"
#include "logs.hpp"
#include "config.h"

#include <string>
#include <sstream>
#include <unordered_map>

extern "C" {
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include "jansson.h"
}

/* Deleter helper-classes for use with std::shared_ptr<> */

class dbh_deleter {
public:
	void operator() (struct sqlite3 *dbh) {
		sqlite3_close(dbh);
	}
};

class hackerboatStateStorage::sth_deleter {
public:
	void operator() (sqlite3_stmt *sth) {
		sqlite3_finalize(sth);
	}
};

hackerboatStateStorage::hackerboatStateStorage(shared_dbh dbh, const char *tableName, std::initializer_list<column> columns)
	: dbh(dbh), tableName(tableName), columns(columns.begin(), columns.end())
{
}

int hackerboatStateStorage::columnCount() const
{
	return columns.size();
}

void hackerboatStateStorage::appendColumns(std::ostringstream& s, bool comma)
{
	for(auto it = columns.cbegin(); it != columns.cend(); it ++) {
		if (comma) {
			s << ", ";
		} else {
			comma = true;
		}
		s << '"' << it->name << '"';
	}
}

void hackerboatStateStorage::prepare(shared_stmt& sth, const std::string& sql)
{
	sqlite3_stmt *prepared_stmt = NULL;
	int err = sqlite3_prepare_v2(dbh.get(), sql.c_str(), sql.length(), &prepared_stmt, NULL);
	if ((err != SQLITE_OK) && (err != SQLITE_BUSY)) {
		logError(sql);
		abort();
	}

	sth.reset(prepared_stmt, sth_deleter());
}

void hackerboatStateStorage::logError(void)
{
	warnx("sqlite error (%p, %s): (%d) %s",
	      dbh.get(), tableName.c_str(),
	      sqlite3_extended_errcode(dbh.get()),
	      sqlite3_errmsg(dbh.get()));
}

void hackerboatStateStorage::logError(const std::string& sql)
{
	warnx("SQL: %s", sql.c_str());
	logError();
}

void hackerboatStateStorage::logError(shared_stmt& sth)
{
	const char *txt = sqlite3_sql(sth.get());
	warnx("SQL: %s", txt);
	logError();
}

static inline int step(shared_stmt& sth)
{
	return sqlite3_step(sth.get());
}

//******************************************************************************
// Helpers to assemble and prepare SQL statements for various operations.

shared_stmt& hackerboatStateStorage::queryLastRecord()
{
	if (!sth_last) {
		std::ostringstream sql;
		sql << "SELECT oid";
		appendColumns(sql, true);
		sql << " FROM \"" << tableName << "\" ORDER BY oid DESC LIMIT 1";

		prepare(sth_last, sql.str());
	} else {
		sqlite3_reset(sth_last.get());
	}

	return sth_last;
}

shared_stmt& hackerboatStateStorage::queryRecord()
{
	if (!sth_byOid) {
		std::ostringstream sql;
		sql << "SELECT ";
		appendColumns(sql, false);
		sql << " FROM \"" << tableName << "\" WHERE OID = ?";

		prepare(sth_byOid, sql.str());
	} else {
		sqlite3_reset(sth_byOid.get());
	}

	return sth_byOid;
}

shared_stmt& hackerboatStateStorage::queryRecordCount()
{
	if (!sth_count) {
		std::ostringstream sql;
		sql << "SELECT COUNT(*) FROM \"" << tableName << '"';
		prepare(sth_count, sql.str());
	} else {
		sqlite3_reset(sth_count.get());
	}

	return sth_count;
}

shared_stmt& hackerboatStateStorage::insertRecord()
{
	if (!sth_insert) {
		std::ostringstream sql;
		sql << "INSERT INTO \"" << tableName << "\" (";
		appendColumns(sql, false);
		sql << ") VALUES (";
		int numPlaceholders = columnCount();
		bool comma = false;
		while(numPlaceholders) {
			if (comma)
				sql << ", ";
			sql << "?";
			comma = true;
			numPlaceholders --;
		}
		sql << ")";

		prepare(sth_insert, sql.str());
	} else {
		sqlite3_reset(sth_insert.get());
	}

	return sth_insert;
}

shared_stmt& hackerboatStateStorage::updateRecord()
{
	if (!sth_update) {
		std::ostringstream sql;
		sql << "UPDATE \"" << tableName << "\" SET ";
		bool comma = false;
		for(auto it = columns.cbegin(); it != columns.cend(); it ++) {
			if (comma) {
				sql << ", ";
			} else {
				comma = true;
			}
			sql << '"' << it->name << "\"=?";
		}
		sql << " WHERE oid = ?";

		prepare(sth_update, sql.str());
	} else {
		sqlite3_reset(sth_update.get());
	}

	return sth_update;
}

//******************************************************************************
// Database connection and schema setup

static std::unordered_map<std::string, shared_dbh> open_files;

static void log_trace(void *ctxt, const char *msg)
{
	std::cerr << "SQL: " << msg << std::endl;
}

static int busy_sleep(void *ctxt, int count)
{
	if (count > 100) {
		return 0;
	}
	if (count > 20) {
		warnx("busy database: sleeping[%d]", count);
	}
	::usleep(count > 10 ? 50000 : 1000);
	return 1;
}

shared_dbh hackerboatStateStorage::databaseConnection(const char *filename)
{
	std::string name( filename? filename : ":memory:" );


	auto curs = open_files.find(name);
	if (curs != open_files.end())
		return curs->second;

	std::string path;

	if (filename && (filename[0] != '/')) {
		const char *db_dir = ::getenv("DB_DIRECTORY");
		if (!db_dir)
			db_dir = DB_DIRECTORY;

		path.append(db_dir);
		if (!path.empty() && path.back() != '/')
			path.append("/");
		path.append(name);
	} else {
		path = name; /* ":memory:" */
	}

	shared_dbh dbh;
	sqlite3 *p;
	if (sqlite3_open_v2(path.c_str(), &p,
			    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
			    NULL) != SQLITE_OK) {
		warnx("sqlite3_open: %s: %s",
		      path.c_str(),
		      p? sqlite3_errmsg(p) : ::strerror(errno));
		sqlite3_close(p /* NULL OK */);
	} else {
		if (::getenv("SQL_TRACE") != NULL) {
			sqlite3_trace(p, log_trace, NULL);  // Trace SQL to std::cerr
		}
		sqlite3_busy_handler(p, busy_sleep, NULL);
		dbh.reset(p, dbh_deleter());
		open_files.emplace(name, dbh);
	}

	return dbh;
}

void hackerboatStateStorage::createTable()
{
	std::ostringstream sql;
	sql << "CREATE TABLE IF NOT EXISTS \"" << tableName << "\" (";
	int numColumns = columnCount();
	for(int i = 0; i < numColumns; i++) {
		if (i > 0) {
			sql << ", ";
		}
		sql << '"' << columns[i].name << "\" " << columns[i].type;
	}
	sql << ")";

	int err = sqlite3_exec(dbh.get(), sql.str().c_str(),
			       NULL, NULL,
			       NULL);
	if ((err != SQLITE_OK) && (err != SQLITE_BUSY)) {
		logError(sql.str());
		abort();
	}
}

void hackerboatStateStorage::closeDatabase (void)
{
	if ((this) && (dbh.get())) {sqlite3_close(dbh.get());}
}

//******************************************************************************
// Methods on storable objects to read and write from the class's hackerboatStateStorage

hackerboatStateClassStorable::sequence hackerboatStateClassStorable::countRecords(void)
{
	hackerboatStateStorage& db = storage();
	shared_stmt sth = db.queryRecordCount();
	int rc = step(sth);
	if (rc != SQLITE_ROW) {
		db.logError(sth);
		sqlite3_reset(sth.get());
		return -1;
	} else {
		hackerboatStateClassStorable::sequence ret = sqlite3_column_int64(sth.get(), 0);
		sqlite3_reset(sth.get());
		return ret;
	}
}

bool hackerboatStateClassStorable::writeRecord(void)
{
	if (_sequenceNum < 0)
		return false;
	hackerboatStateStorage& db = storage();
	shared_stmt sth = db.updateRecord();
	int columnCount = db.columnCount();
	assert(sqlite3_bind_parameter_count(sth.get()) == columnCount+1);
	if (!fillRow(sqliteParameterSlice(sth, 1, columnCount))) {
		//sqlite3_reset(sth.get());
		return false;
	}
	sqlite3_bind_int64(sth.get(), columnCount+1, _sequenceNum);
	int rc = step(sth);
	if (rc == SQLITE_DONE) {
		//sqlite3_reset(sth.get());
		return true;
	} else {
		db.logError(sth);
		//sqlite3_reset(sth.get());
		return false;
	}
}

bool hackerboatStateClassStorable::appendRecord(void)
{
	hackerboatStateStorage& db = storage();
	shared_stmt sth = db.insertRecord();
	assert(sqlite3_bind_parameter_count(sth.get()) == db.columnCount());
	sqliteParameterSlice parameters(sth, 1, db.columnCount());
	if (!fillRow(parameters))
		//sqlite3_reset(sth.get());
		return false;
	int rc = step(sth);
	if (rc == SQLITE_DONE) {
		_sequenceNum = sqlite3_last_insert_rowid(db.dbh.get());
		//sqlite3_reset(sth.get());
		return true;
	} else {
		db.logError(sth);
		//sqlite3_reset(sth.get());
		return false;
	}
}

bool hackerboatStateClassStorable::getLastRecord(void)
{
	hackerboatStateStorage& db = storage();
	shared_stmt sth = db.queryLastRecord();
	int rc = step(sth);
	if (rc == SQLITE_ROW) {
		bool ret = readFromRow(sqliteRowReference(sth, 1, db.columnCount()),
							sqlite3_column_int64(sth.get(), 0));
		sqlite3_reset(sth.get());
		return ret;
	} else if (rc == SQLITE_DONE) {
		/* No rows returned: the table is empty. */
		sqlite3_reset(sth.get());
		return false;
	} else {
		db.logError(sth);
		sqlite3_reset(sth.get());
		return false;
	}
}

bool hackerboatStateClassStorable::getRecord(sequence oid)
{
	hackerboatStateStorage& db = storage();
	shared_stmt sth = db.queryRecord();
	assert(sqlite3_column_count(sth.get()) == db.columnCount());
	sqlite3_bind_int64(sth.get(), 1, oid);
	int rc = step(sth);
	if (rc == SQLITE_ROW) {
		bool ret = readFromRow(sqliteRowReference(sth, 0, db.columnCount()), oid);
		sqlite3_reset(sth.get());
		return ret;
	} else if (rc == SQLITE_DONE) {
		/* No rows returned: the table is empty. */
		sqlite3_reset(sth.get());
		return false;
	} else {
		db.logError(sth);
		sqlite3_reset(sth.get());
		return false;
	}
}

//******************************************************************************
// C++-y helper classes for parameter and row slices

void sqliteParameterSlice::bind_json_new(int column, json_t *j)
{
	assert(column >= 0);
	assert(column < count);
	if (json_is_null(j)) {
		sqlite3_bind_null(sth, column + offset);
	} else {
		char *encoded = json_dumps(j, JSON_COMPACT);
		sqlite3_bind_text(sth, column + offset, encoded, -1, ::free);
	}
	json_decref(j);
}

json_t *sqliteRowReference::json_field(int column) const
{
	assert(column >= 0);
	assert(column < count);
	int coltype = sqlite3_column_type(sth, column + offset);
	if (coltype == SQLITE_NULL) {
		return json_null();
	}
	int length = sqlite3_column_bytes(sth, column + offset);
	const unsigned char *contents = sqlite3_column_text(sth, column + offset);
	return json_loadb(reinterpret_cast<const char *>(contents), length, 0, NULL);
}

std::string sqliteRowReference::string_field(int column) const
{
	assert(column >= 0);
	assert(column < count);
	if (sqlite3_column_type(sth, column + offset) == SQLITE_NULL) {
		// ...;
	}
	int length = sqlite3_column_bytes(sth, column + offset);
	const unsigned char *contents = sqlite3_column_text(sth, column + offset);
	return std::string(reinterpret_cast<const char *>(contents), length);
}

