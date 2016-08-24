
#include "hackerboatRoot.hpp"
#include "orientation.hpp"
#include "location.hpp"
#include "navigation.hpp"
#include "test_utilities.hpp"
#include <gtest/gtest.h>

TEST(Serialization, Orientation) {

	json_t *sample = json_loads("{ \"roll\": 0.0, \"pitch\": -1, \"yaw\": 144.5 }", 0, NULL);
	ASSERT_TRUE(sample);

	Orientation o(3,4,5);
	ASSERT_TRUE(o.parse(sample));

	ASSERT_EQ(o.roll, 0);
	ASSERT_EQ(o.pitch, -1.0);
	ASSERT_EQ(o.heading, 144.5);

	json_t *roundtrip = o.pack();

	EXPECT_JSON_EQ(sample, roundtrip);

	json_decref(sample);
	json_decref(roundtrip);
}


TEST(Serialization, NavVector) {
	navVectorClass v1;
	EXPECT_FALSE(v1.isValid());

	navVectorClass v2("fnord", 1, 2);
	EXPECT_TRUE(v2.isValid());
	json_t *packed = v2.pack();
	ASSERT_TRUE(packed);

	json_t *expected = json_loads("{ \"source\": \"fnord\", \"bearing\": 1.0, \"strength\": 2.0 }", 0, NULL);
	ASSERT_TRUE(expected);
	EXPECT_JSON_EQ(expected, packed);

	EXPECT_TRUE(v1.parse(packed));
	EXPECT_TRUE(v1.isValid());
	EXPECT_EQ(v1._source, std::string("fnord"));
	EXPECT_EQ(v1._bearing, 1.0);
	EXPECT_EQ(v1._strength, 2.0);

	json_decref(packed);
	json_decref(expected);
}

TEST(Serialization, Location) {
	Location loc;
	EXPECT_FALSE(loc.isValid());

	json_t *sample = json_loads("{ \"latitude\": 47.58, \"longitude\": -122.05 }", 0, NULL);

	ASSERT_TRUE(loc.parse(sample));

	EXPECT_TRUE(loc.isValid());
	ASSERT_EQ(loc._lat, 47.58);
	ASSERT_EQ(loc._lon, -122.05);

	json_t *roundtrip = loc.pack();
	ASSERT_TRUE(roundtrip);

	EXPECT_JSON_EQ(sample, roundtrip);

	json_decref(sample);
	json_decref(roundtrip);
}

