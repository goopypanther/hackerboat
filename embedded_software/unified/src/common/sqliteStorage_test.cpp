

#include <gtest/gtest.h>
#include "sqliteStorage.hpp"
extern "C" {
#include <sqlite3.h>
}
#include "hackerboatRoot.hpp"

static bool table_exists(shared_dbh& dbh, const char *tablename)
{
	sqlite3_stmt *sth = NULL;
	int err = sqlite3_prepare(dbh.get(), "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?", -1, &sth, NULL);
	EXPECT_EQ(err, SQLITE_OK);
	if (err != SQLITE_OK)
		return false;
	sqlite3_bind_text(sth, 1, tablename, -1, SQLITE_STATIC);
	err = sqlite3_step(sth);
	EXPECT_EQ(err, SQLITE_ROW);
	if (err != SQLITE_ROW)
		return false;
	int count = sqlite3_column_int(sth, 0);
	sqlite3_finalize(sth);

	EXPECT_TRUE(count == 0 || count == 1);

	return count > 0;
}


TEST(StorageTest, Creation) {

	shared_dbh dbh = hackerboatStateStorage::databaseConnection("test0");
	ASSERT_TRUE(bool(dbh));

	if (table_exists(dbh, "FOO")) {
		int err = sqlite3_exec(dbh.get(), "DROP TABLE FOO", NULL, NULL, NULL);
		EXPECT_EQ(err, SQLITE_OK);
		EXPECT_FALSE(table_exists(dbh, "FOO"));
	}

	hackerboatStateStorage storage(dbh, "FOO", { { "thing", "INTEGER" },
						     { "whatsit", "TEXT" } });

	EXPECT_FALSE(table_exists(dbh, "FOO"));
	storage.createTable();
	EXPECT_TRUE(table_exists(dbh, "FOO"));
}

class simpleValues : public hackerboatStateClassStorable {

public:
	simpleValues(shared_dbh& dbh);

	int i;
	double f;
	std::string s;

	/* Stub impls */
	bool parse(json_t* s, bool q) {
		return false;
	}
	bool isValid() const {
		return true;
	}
	json_t *pack(bool q) const {
		return NULL;
	}

protected:
	hackerboatStateStorage _storage;
	hackerboatStateStorage& storage(void);
	bool fillRow(sqliteParameterSlice) const;
	bool readFromRow(sqliteRowReference, sequence);
};

simpleValues::simpleValues(shared_dbh& dbh)
	: _storage(dbh, "SIMPLEVALUES",
		   { { "i", "INTEGER" },
		     { "f", "REAL" },
		     { "s", "TEXT" } })
{
	_storage.createTable();
}

hackerboatStateStorage& simpleValues::storage()
{
	return _storage;
}

bool simpleValues::fillRow(sqliteParameterSlice row) const
{
	row.bind(0, i);
	row.bind(1, f);
	row.bind(2, s);
	return true;
}

bool simpleValues::readFromRow(sqliteRowReference row, sequence seq)
{
	_sequenceNum = seq;
	i = row.int64_field(0);
	f = row.double_field(1);
	s = row.string_field(2);
	return true;
}

TEST(StorageTest, SimpleValues) {
	shared_dbh dbh = hackerboatStateStorage::databaseConnection("test0");
	ASSERT_TRUE(bool(dbh));

	simpleValues::sequence oid1, oid2;

	simpleValues::sequence count0, count1, count2;

	{
		simpleValues row1(dbh);
		row1.i = 42;
		row1.f = 4.25;
		row1.s = "Hoopy Frood";

		count0 = row1.countRecords();
		EXPECT_TRUE(count0 >= 0);

		EXPECT_EQ(row1.getSequenceNum(), -1);
		row1.appendRecord();
		EXPECT_NE(row1.getSequenceNum(), -1);
		oid1 = row1.getSequenceNum();
	}

	{
		simpleValues row1(dbh);
		row1.getLastRecord();
		EXPECT_EQ(row1.getSequenceNum(), oid1);

		EXPECT_EQ(row1.i, 42);
		EXPECT_EQ(row1.f, 4.25);
		EXPECT_EQ(row1.s, "Hoopy Frood");

		count1 = row1.countRecords();
		EXPECT_EQ(count1, count0 + 1);

		simpleValues row2(dbh);
		row2.i = 23;
		row2.f = 7;
		row2.s = "Sacred Chao";

		EXPECT_EQ(row2.getSequenceNum(), -1);
		row2.appendRecord();
		oid2 = row2.getSequenceNum();
		EXPECT_NE(oid2, -1);
		EXPECT_TRUE(oid2 > oid1);

		count2 = row1.countRecords();
		EXPECT_EQ(count2, count1 + 1);

		row1.f = 42.5;
		row1.writeRecord();
		EXPECT_EQ(row1.getSequenceNum(), oid1);

		EXPECT_EQ(row2.countRecords(), count2);
	}


	{
		simpleValues row1(dbh), row2(dbh);

		row1.getRecord(oid1);
		row2.getLastRecord();

		EXPECT_EQ(row1.getSequenceNum(), oid1);
		EXPECT_EQ(row2.getSequenceNum(), oid2);

		EXPECT_EQ(row1.i, 42);
		EXPECT_EQ(row1.f, 42.5);
		EXPECT_EQ(row1.s, "Hoopy Frood");

		EXPECT_EQ(row2.i, 23);
		EXPECT_EQ(row2.f, 7);
		EXPECT_EQ(row2.s, "Sacred Chao");

		EXPECT_EQ(row2.countRecords(), count2);
	}
}
