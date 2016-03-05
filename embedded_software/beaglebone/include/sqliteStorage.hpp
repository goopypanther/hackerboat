/******************************************************************************
 * Hackerboat Beaglebone SQLite storage adapter
 *
 * see the Hackerboat documentation for more details
 * Written by Wim Lewis, Mar 2016
 * 
 * Version 0.1: 
 *
 ******************************************************************************/

#ifndef STORAGE_H
#define STORAGE_H

extern "C" {
#include <sqlite3.h>
}

#include <memory>
#include <sstream>
#include <vector>

typedef std::shared_ptr<sqlite3> shared_dbh;
typedef std::shared_ptr<sqlite3_stmt> shared_stmt;

/**
 * Represents a database connection and schema information
 *
 */
class hackerboatStateStorage {
public:
	hackerboatStateStorage(shared_dbh dbh, const char *tableName, std::vector<const char*> columns)
		: dbh(dbh), tableName(tableName), columns(columns)
	{};

	shared_dbh dbh;
	const std::string tableName;
	const std::vector<const char *> columns;
	int columnCount() const;

	shared_stmt& queryLastRecord();
	shared_stmt& queryRecord(int32_t oid);
	shared_stmt& queryRecordCount();
	shared_stmt& insertRecord();
	shared_stmt& updateRecord();

	void createTable(const char * const *);
	void logError(void);

	static shared_dbh databaseConnection(const char *filename);
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

	sqliteParameterSlice slice(int sliceOffset, int sliceCount) {
		return sqliteParameterSlice(sth, offset + sliceOffset, sliceCount);
	}

	void bind(int column, double value) {
		sqlite3_bind_double(sth, column + offset, value);
	}
	void bind(int column, int value) {
		sqlite3_bind_int(sth, column + offset, value);
	}
	void bind(int column, int64_t value) {
		sqlite3_bind_int64(sth, column + offset, value);
	}
	void bind(int column, std::string& value) {
		sqlite3_bind_text(sth, column + offset, value.data(), value.length(), SQLITE_TRANSIENT);
	}
	void bind_json_new(int column, struct json_t *value);
	void bind_null(int column) {
		sqlite3_bind_null(sth, column + offset);
	}
};

/**
 * Encapsulates a statement (result) handle and an offset-and-range into its column list
 */
class sqliteRowReference {
protected:
	const int offset;
	sqlite3_stmt * const sth;

	sqliteRowReference(sqlite3_stmt *sth, int offset, int count)
		: sth(sth), offset(offset)
	{};

public:
	sqliteRowReference(shared_stmt sth, int offset, int count)
		: sth(sth.get()), offset(offset)
	{};
	
	sqliteRowReference slice(int sliceOffset, int sliceCount) {
		return sqliteRowReference(sth, offset + sliceOffset, sliceCount);
	}

	struct json_t *json_field(int column) const;
	int64_t int64_field(int column) const {
		return sqlite3_column_int64(sth, column + offset);
	}
	double double_field(int column) const {
		return sqlite3_column_double(sth, column + offset);
	}
};

#endif

