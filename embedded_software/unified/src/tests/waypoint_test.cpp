#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "waypoint.hpp"
#include "twovector.hpp"
#include "test_utilities.hpp"

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
	mywaypoint->loadKML();
	EXPECT_EQ(mywaypoint->count(), 7);
	EXPECT_EQ(mywaypoint->current(), 0);
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::NONE);
}

TEST_F (WaypointTest, SetNumber) {
	mywaypoint->loadKML();
	EXPECT_EQ(mywaypoint->count(), 7);
	mywaypoint->setCurrent(3);
	EXPECT_EQ(mywaypoint->current(), 3);
	mywaypoint->setCurrent(7);
	EXPECT_EQ(mywaypoint->current(), 6);
	mywaypoint->setCurrent(9);
	EXPECT_EQ(mywaypoint->current(), 6);
	mywaypoint->setCurrent(1);
	EXPECT_EQ(mywaypoint->current(), 1);
	mywaypoint->setCurrent(-1);
	EXPECT_EQ(mywaypoint->current(), 6);
}

TEST_F (WaypointTest, ParseCheck) {
	ASSERT_TRUE(mywaypoint->loadKML());
	Location wp;
	EXPECT_EQ(mywaypoint->count(), 7);
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3525047, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5989868, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3749924, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6215539, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6334701, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6015333, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3753357, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6045427, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3791122, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5943564, TOL));
	wp = mywaypoint->getWaypoint(0);
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
}

TEST_F (WaypointTest, ReloadCheck) {
	Location wp;
	EXPECT_FALSE(mywaypoint->loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_3.kml"));
	EXPECT_TRUE(mywaypoint->loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_2.kml"));
	EXPECT_EQ(mywaypoint->count(), 7);
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3799706, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5906518, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3525047, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5989868, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6015333, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3753357, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6045427, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3749924, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6215539, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.4834824, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.6334701, TOL));
	EXPECT_TRUE(mywaypoint->increment());
	wp = mywaypoint->getWaypoint();
	EXPECT_TRUE(toleranceEquals(wp.lon, -122.3791122, TOL));
	EXPECT_TRUE(toleranceEquals(wp.lat, 47.5943564, TOL));
}

TEST_F (WaypointTest, GetSetAction) {
	ASSERT_TRUE(mywaypoint->loadKML());
	Location wp;
	mywaypoint->setAction(WaypointActionEnum::IDLE);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::IDLE);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_FALSE(mywaypoint->decrement());
	mywaypoint->setAction(WaypointActionEnum::ANCHOR);
	mywaypoint->setCurrent(6);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::ANCHOR);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 6);
	mywaypoint->setAction(WaypointActionEnum::RETURN);
	mywaypoint->setCurrent(6);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::RETURN);
	EXPECT_FALSE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 6);
	mywaypoint->setAction(WaypointActionEnum::REPEAT);
	mywaypoint->setCurrent(6);
	EXPECT_TRUE(mywaypoint->getWaypoint().isValid());
	EXPECT_EQ(mywaypoint->getAction(), WaypointActionEnum::REPEAT);
	EXPECT_TRUE(mywaypoint->increment());
	EXPECT_EQ(mywaypoint->current(), 0);
}


