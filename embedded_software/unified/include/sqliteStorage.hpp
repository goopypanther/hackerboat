/**************************************************************************//**
 * Hackerboat Beaglebone SQLite storage adapter
 *
 * @file sqliteStorage.hpp
 * @author Wim Lewis
 *
 ******************************************************************************/

#ifndef STORAGE_H
#define STORAGE_H

extern "C" {
#include <sqlite3.h>
#include <assert.h>
}

#include <memory>
#include <sstream>
#include <vector>
#include <initializer_list>

typedef std::shared_ptr<sqlite3> shared_dbh;
typedef std::shared_ptr<sqlite3_stmt> shared_stmt;

/**
 * Represents a database connection and schema information
 *
 * This contains a database handle, cached SQLite queries, and enough
 * information about the table schema to create queries and to create
 * the initial empty table in the database.
 *
 * This is used by hackerboatStateClassStorable to write objects to
 * persistent storage.
 */
class hackerboatStateStorage {
public:
	struct column {
		const char *name;
		const char *type;
	};

	hackerboatStateStorage(shared_dbh dbh, const char *tableName, std::initializer_list<column> columns);

	shared_dbh dbh;
	const std::string tableName;
	const std::vector<column> columns;
	int columnCount() const;

	shared_stmt& queryLastRecord();
	shared_stmt& queryRecord();
	shared_stmt& queryRecordCount();
	shared_stmt& insertRecord();
	shared_stmt& updateRecord();

	/** Create the table in the database.
	 * This issues a <tt>CREATE TABLE IF NOT EXISTS</tt> query, so it is a no-op if the table already exists.
	 */
	void createTable();

	void logError(void);
	void logError(const std::string& sql);
	void logError(shared_stmt& sql);

	/** Gets an open connection to the database
	 *
	 * This maintains a table of filenames to open database handles
	 * and returns a shared reference to an existing handle, or a new
	 * handle. The directory in which database files are stored is
	 * defined in config.h.
	 */
	static shared_dbh databaseConnection(const char *filename);
	void closeDatabase (void);
	
protected:
	class sth_deleter;
	class stmt_deleter;
	void appendColumns(std::ostringstream& s, bool comma = true);
	void prepare(shared_stmt& sth, const std::string& sql);
private:
	shared_stmt sth_last, sth_byOid, sth_count, sth_insert, sth_update;
};

/**
 * Encapsulates a statement handle and an offset-and-range into its parameter list
 */
class sqliteParameterSlice {
protected:
	const int offset;
	sqlite3_stmt * const sth;

	sqliteParameterSlice(sqlite3_stmt *sth, int offset, int count)
		: sth(sth), offset(offset), count(count)
	{};

public:
	const int count;

	sqliteParameterSlice(shared_stmt sth, int offset, int count)
		: sth(sth.get()), offset(offset), count(count)
	{};

	/** Return a sub-slice */
	sqliteParameterSlice slice(int sliceOffset, int sliceCount) {
		return sqliteParameterSlice(sth, offset + sliceOffset, sliceCount);
	}

	void assertWidth(int width) {
		assert (count == width);
	}

	void bind(int column, double value) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_double(sth, column + offset, value);
	}
	void bind(int column, int value) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_int(sth, column + offset, value);
	}
	void bind(int column, int64_t value) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_int64(sth, column + offset, value);
	}
	void bind(int column, bool value) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_int64(sth, column + offset, value? 1 : 0);
	}
	void bind(int column, const std::string& value) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_text(sth, column + offset, value.data(), value.length(), SQLITE_TRANSIENT);
	}
	void bind_json_new(int column, struct json_t *value);
	void bind_null(int column) {
		assert(column >= 0);
		assert(column < count);
		sqlite3_bind_null(sth, column + offset);
	}
};

/**
 * Encapsulates a statement (result) handle and an offset-and-range into its column list
 */
class sqliteRowReference {
protected:
	const int offset;
#ifndef NDEBUG
	const int count;
#endif
	sqlite3_stmt * const sth;

	sqliteRowReference(sqlite3_stmt *sth, int offset, int count)
		: sth(sth), offset(offset)
#ifndef NDEBUG
		, count(count)
#endif
	{};

public:
	sqliteRowReference(shared_stmt sth, int offset, int count)
		: sth(sth.get()), offset(offset)
#ifndef NDEBUG
		, count(count)
#endif
	{};
	
	sqliteRowReference slice(int sliceOffset, int sliceCount) const {
		return sqliteRowReference(sth, offset + sliceOffset, sliceCount);
	}

	void assertWidth(int width) {
		assert (count == width);
	}

	struct json_t *json_field(int column) const;
	int64_t int64_field(int column) const {
		assert(column >= 0);
		assert(column < count);
		return sqlite3_column_int64(sth, column + offset);
	}
	double double_field(int column) const {
		assert(column >= 0);
		assert(column < count);
		return sqlite3_column_double(sth, column + offset);
	}
	bool isnull(int column) const {
		assert(column >= 0);
		assert(column < count);
		return sqlite3_column_type(sth, column + offset) == SQLITE_NULL;
	}
	bool bool_field(int column) const {
		assert(column >= 0);
		assert(column < count);
		return sqlite3_column_int64(sth, column + offset) ? true : false;
	}
	std::string string_field(int column) const;
};

#endif

