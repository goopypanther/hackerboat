
#include "navigation.hpp"
#include "test_utilities.hpp"
#include <gtest/gtest.h>

TEST(Serialization, NavClass) {

	const char *s =
	"{ "
	" \"current\": { \"latitude\": 54.5, \"longitude\": -100.5 }, "
	" \"target\": { \"latitude\": 53, \"longitude\": -108.25, \"index\": 3, \"nextWaypoint\": 4, \"action\": \"CONTINUE\"  }, "
	" \"waypointStrength\": 0.125, "
	" \"magCorrection\": -1.25, "
	" \"targetVec\": { \"source\": \"\", \"bearing\": 108, \"strength\": 0.0 }, "
	" \"total\": { \"source\": \"\", \"bearing\": 95.5, \"strength\": 1.0 }, "
	" \"navInfluences\": [ ] "
	" }";

	json_t *sample = json_loads(s, 0, NULL);
	ASSERT_TRUE(sample);

	navClass n;
	EXPECT_TRUE(n.parse(sample, false));

	json_t *roundtrip = n.pack(false);
	EXPECT_JSON_EQ(sample, roundtrip);
	json_decref(roundtrip);

	json_string_set(json_object_get(json_object_get(sample, "target"), "action"), "HOME");
	n.target.setAction(waypointClass::action::HOME);
	roundtrip = n.pack(false);
	EXPECT_JSON_EQ(sample, roundtrip);
	json_decref(roundtrip);

	json_decref(sample);
}


