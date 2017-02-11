#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "waypoint.hpp"
#include "twovector.hpp"
#include "test_utilities.hpp"
#include "easylogging++.h"

#define TOL 0.000001

class WaypointTest : public ::testing::Test {
	public:
	
		Waypoints *mywaypoint;
	
		WaypointTest () {
			mywaypoint = new Waypoints("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
		};
		
		~WaypointTest () {
			delete mywaypoint;
		};
};

TEST_F (WaypointTest, Creation) {
	VLOG(1) << "===Waypoint Creation Test===";
	bool result = mywaypoint->loadKML();
	ASSERT_TRUE(result);
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	EXPECT_EQ(mywaypoint->count(), 7);
	VLOG(2) << "Loaded " << mywaypoint->count() << " waypoints";
	EXPECT_EQ(mywaypoint->current(), 0);
	VLOG(2) << "Current waypoint is " << mywaypoint->current();
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::NONE);
	VLOG(2) << "Waypoint action is " << Waypoints::actionNames.get(mywaypoint->getAction());
}

TEST_F (WaypointTest, SetNumber) {
	VLOG(1) << "===Waypoint Number Setting Test===";
	bool result = mywaypoint->loadKML();
	ASSERT_TRUE(result);
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	EXPECT_EQ(mywaypoint->count(), 7);
	VLOG(2) << "Loaded " << mywaypoint->count() << " waypoints";
	mywaypoint->setCurrent(3);
	VLOG(2) << "Setting current waypoint to 3... ";
	EXPECT_EQ(mywaypoint->current(), 3);
	VLOG(2) << mywaypoint->current();
	mywaypoint->setCurrent(7);
	VLOG(2) << "Setting current waypoint to 7... ";
	EXPECT_EQ(mywaypoint->current(), 6);
	VLOG(2) << mywaypoint->current();
	mywaypoint->setCurrent(9);
	VLOG(2) << "Setting current waypoint to 9... ";
	EXPECT_EQ(mywaypoint->current(), 6);
	VLOG(2) << mywaypoint->current();
	mywaypoint->setCurrent(1);
	VLOG(2) << "Setting current waypoint to 1... ";
	EXPECT_EQ(mywaypoint->current(), 1);
	VLOG(2) << mywaypoint->current();
	mywaypoint->setCurrent(-1);
	VLOG(2) << "Setting current waypoint to -1... ";
	EXPECT_EQ(mywaypoint->current(), 6);
	VLOG(2) << mywaypoint->current();
}

TEST_F (WaypointTest, ParseCheck) {
	VLOG(1) << "===KML Parsing Test===";
	bool result = mywaypoint->loadKML();
	ASSERT_TRUE(result);
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	Location wp;
	EXPECT_EQ(mywaypoint->count(), 7);
	VLOG(2) << "Loaded " << mywaypoint->count() << " waypoints";
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3525047, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5989868, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3749924, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6215539, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6334701, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6015333, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3753357, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6045427, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3791122, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5943564, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	wp = mywaypoint->getWaypoint(0);
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
	VLOG(2) << "Waypoint 0 is " << json_dumps(wp.pack(), 0);
}

TEST_F (WaypointTest, ReloadCheck) {
	VLOG(1) << "=== KML Reload Test===";
	Location wp;
	bool result = mywaypoint->loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_3.kml");
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	EXPECT_FALSE(result);
	result = mywaypoint->loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_2.kml");
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	EXPECT_TRUE(result);
	EXPECT_EQ(mywaypoint->count(), 7);
	VLOG(2) << "Loaded " << mywaypoint->count() << " waypoints";
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3525047, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5989868, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6015333, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3753357, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6045427, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3749924, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6215539, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6334701, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3791122, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5943564, TOL));
	VLOG(2) << "Waypoint " << mywaypoint->current() << " is " << json_dumps(wp.pack(), 0);
}

TEST_F (WaypointTest, GetSetAction) {
	VLOG(1) << "===Waypoint Action Test===";
	bool result = mywaypoint->loadKML();
	VLOG_IF(result, 1) << "Loaded KML file " << mywaypoint->getKMLPath();
	VLOG_IF(!result, 1) << "Failed to load KML file" << mywaypoint->getKMLPath();
	ASSERT_TRUE(result);
	Location wp;
	mywaypoint->setAction(WaypointActionEnum::IDLE);
	VLOG(2) << "Waypoint action is " << Waypoints::actionNames.get(mywaypoint->getAction());
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::IDLE);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_FALSE(mywaypoint->decrement());
	mywaypoint->setAction(WaypointActionEnum::ANCHOR);
	VLOG(2) << "Waypoint action is " << Waypoints::actionNames.get(mywaypoint->getAction());
	mywaypoint->setCurrent(6);
	VLOG(2) << "Current waypoint is " << mywaypoint->current();
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::ANCHOR);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 6);
	VLOG(2) << "Current waypoint is " << mywaypoint->current();
	mywaypoint->setAction(WaypointActionEnum::RETURN);
	VLOG(2) << "Waypoint action is " << Waypoints::actionNames.get(mywaypoint->getAction());
	mywaypoint->setCurrent(6);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::RETURN);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 6);
	VLOG(2) << "Current waypoint is " << mywaypoint->current();
	mywaypoint->setAction(WaypointActionEnum::REPEAT);
	VLOG(2) << "Waypoint action is " << Waypoints::actionNames.get(mywaypoint->getAction());
	mywaypoint->setCurrent(6);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::REPEAT);
	EXPECT_TRUE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 0);
	VLOG(2) << "Current waypoint is " << mywaypoint->current();
}


