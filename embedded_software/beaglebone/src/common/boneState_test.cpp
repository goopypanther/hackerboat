
#include "stateStructTypes.hpp"
#include "boneState.hpp"
#include "test_utilities.hpp"
#include <gtest/gtest.h>

TEST(Serialization, BoneState) {

	const char *s =
	"{ "
	" \"uTime\": { \"tv_sec\": 987654321, \"tv_nsec\": 123312 }, "
	" \"lastContact\": { \"tv_sec\": 123456789, \"tv_nsec\": 986788 }, "
	" \"mode\": \"SelfTest\", "
	" \"command\": \"WaypointNavigation\", "
	" \"ardMode\": \"SelfTest\", "

	" \"faultString\": \"howdy,pardner\", "

	" \"waypointNext\": 14, "
	" \"waypointStrength\": 1.0, "
	" \"waypointAccuracy\": 12, "
	" \"waypointStrengthMax\": 1000, "
	" \"autonomous\": false, "
	" \"launchPoint\": { \"latitude\": 42, \"longitude\": -122 } "
	" }";

	const char *g =
	"{ "
	" \"uTime\": { \"tv_sec\": 987654321, \"tv_nsec\": 123312 }, "
	" \"latitude\": 42.5, \"longitude\": -122.75, "
	" \"heading\": 18.5, "
	" \"speed\": 0.75, "
	" \"fixValid\": true "
	"}";

	json_t *sample = json_loads(s, 0, NULL);
	ASSERT_TRUE(sample);
	json_object_set_new(sample, "gps", json_loads(g, 0, NULL));

	boneStateClass b;
	ASSERT_TRUE(b.parse(sample, false));
	
	json_t *roundtrip = b.pack(false);
	EXPECT_JSON_EQ(sample, roundtrip);
	json_decref(roundtrip);

	json_decref(sample);
}


