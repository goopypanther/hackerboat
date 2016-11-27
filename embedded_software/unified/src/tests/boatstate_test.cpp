#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "boatState.hpp"
#include "enumdefs.hpp"
#include "test_utilities.hpp"
#include <jansson.h>

#define TOL 0.000001

TEST(BoatStateTest, FaultString) {
	BoatState me;
	EXPECT_EQ(me.faultCount(), 0);
	EXPECT_TRUE(me.insertFault("one"));
	EXPECT_EQ(me.faultCount(), 1);
	EXPECT_TRUE(me.insertFault("one"));
	EXPECT_EQ(me.faultCount(), 1);
	EXPECT_TRUE(me.insertFault("two"));
	EXPECT_EQ(me.faultCount(), 2);
	EXPECT_TRUE(me.hasFault("one"));
	EXPECT_TRUE(me.hasFault("two"));
	EXPECT_FALSE(me.hasFault("three"));
	EXPECT_TRUE(me.removeFault("two"));
	EXPECT_FALSE(me.hasFault("two"));
	EXPECT_EQ(me.faultCount(), 1);
	me.clearFaults();
	EXPECT_FALSE(me.hasFault("one"));
	EXPECT_EQ(me.faultCount(), 0);
}

TEST(BoatStateTest, BoatMode) {
	BoatState me;
	EXPECT_FALSE(me.setBoatMode(""));
	EXPECT_TRUE(me.setBoatMode("Start"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_TRUE(me.setBoatMode("SelfTest"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::SELFTEST);
	EXPECT_TRUE(me.setBoatMode("Disarmed"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_TRUE(me.setBoatMode("Fault"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::FAULT);
	EXPECT_TRUE(me.setBoatMode("Navigation"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NAVIGATION);
	EXPECT_TRUE(me.setBoatMode("ArmedTest"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_TRUE(me.setBoatMode("None"));
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NONE);
}

TEST(BoatStateTest, NavMode) {
	BoatState me;
	EXPECT_FALSE(me.setNavMode(""));
	EXPECT_TRUE(me.setNavMode("Idle"));
	EXPECT_EQ(me.getNavMode(), NavModeEnum::IDLE);
	EXPECT_TRUE(me.setNavMode("Fault"));
	EXPECT_EQ(me.getNavMode(), NavModeEnum::FAULT);
	EXPECT_TRUE(me.setNavMode("RC"));
	EXPECT_EQ(me.getNavMode(), NavModeEnum::RC);
	EXPECT_TRUE(me.setNavMode("Autonomous"));
	EXPECT_EQ(me.getNavMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_TRUE(me.setNavMode("None"));
	EXPECT_EQ(me.getNavMode(), NavModeEnum::NONE);
}

TEST(BoatStateTest, AutoMode) {
	BoatState me;
	EXPECT_FALSE(me.setAutoMode(""));
	EXPECT_TRUE(me.setAutoMode("Idle"));
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::IDLE);
	EXPECT_TRUE(me.setAutoMode("Waypoint"));
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::WAYPOINT);
	EXPECT_TRUE(me.setAutoMode("Return"));
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::RETURN);
	EXPECT_TRUE(me.setAutoMode("Anchor"));
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::ANCHOR);
	EXPECT_TRUE(me.setAutoMode("None"));
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::NONE);
}

TEST(BoatStateTest, RCMode) {
	BoatState me;
	EXPECT_FALSE(me.setRCmode(""));
	EXPECT_TRUE(me.setRCmode("Idle"));
	EXPECT_EQ(me.getRCMode(), RCModeEnum::IDLE);
	EXPECT_TRUE(me.setRCmode("Rudder"));
	EXPECT_EQ(me.getRCMode(), RCModeEnum::RUDDER);
	EXPECT_TRUE(me.setRCmode("Course"));
	EXPECT_EQ(me.getRCMode(), RCModeEnum::COURSE);
	EXPECT_TRUE(me.setRCmode("Failsafe"));
	EXPECT_EQ(me.getRCMode(), RCModeEnum::FAILSAFE);
	EXPECT_TRUE(me.setRCmode("None"));
	EXPECT_EQ(me.getRCMode(), RCModeEnum::NONE);
}

TEST(BoatStateTest, JSON) {
	BoatState me, you;
	json_error_t *err;
	GPSFix testfix;
	testfix.fix = Location(5.0, 5.0);
	me.relays = RelayMap::instance();
	me.recordTime = std::chrono::system_clock::now();
	me.currentWaypoint = 4;
	me.waypointStrength = 20.0;
	me.lastContact = std::chrono::system_clock::now();
	me.lastRC = std::chrono::system_clock::now();
	me.launchPoint = Location(10.0, 10.0);
	me.lastFix = testfix;
	me.insertFault("test");
	me.setBoatMode("Disarmed");
	me.setNavMode("Fault");
	me.setAutoMode("Waypoint");
	me.setRCmode("Rudder");
	json_t* packed;
	ASSERT_NO_THROW(packed = me.pack());
	ASSERT_TRUE(packed);
	EXPECT_TRUE(you.parse(packed));
	EXPECT_EQ(me.recordTime, you.recordTime);
	EXPECT_EQ(me.currentWaypoint, you.currentWaypoint);
	EXPECT_EQ(me.waypointStrength, you.waypointStrength);
	EXPECT_EQ(me.lastContact, you.lastContact);
	EXPECT_EQ(me.lastRC, you.lastRC);
	EXPECT_EQ(me.launchPoint.lat, you.launchPoint.lat);
	EXPECT_EQ(me.launchPoint.lon, you.launchPoint.lon);
	EXPECT_EQ(me.lastFix.fix.lat, you.lastFix.fix.lat);
	EXPECT_EQ(me.lastFix.fix.lon, you.lastFix.fix.lon);
	EXPECT_TRUE(you.hasFault("test"));
	EXPECT_EQ(me.getBoatMode(), you.getBoatMode());
	EXPECT_EQ(me.getNavMode(), you.getNavMode());
	EXPECT_EQ(me.getAutoMode(), you.getAutoMode());
	EXPECT_EQ(me.getRCMode(), you.getRCMode());
}

TEST(BoatStateTest, Storage) {
	BoatState me, you;
	json_error_t *err;
	GPSFix testfix;
	testfix.fix = Location(5.0, 5.0);
	me.relays = RelayMap::instance();
	me.recordTime = std::chrono::system_clock::now();
	me.currentWaypoint = 4;
	me.waypointStrength = 20.0;
	me.lastContact = std::chrono::system_clock::now();
	me.lastRC = std::chrono::system_clock::now();
	me.launchPoint = Location(10.0, 10.0);
	me.lastFix = testfix;
	me.insertFault("test");
	me.setBoatMode("Disarmed");
	me.setNavMode("Fault");
	me.setAutoMode("Waypoint");
	me.setRCmode("Rudder");
	EXPECT_GT(me.countRecords(), -1);
	EXPECT_TRUE(me.appendRecord());
	EXPECT_TRUE(you.getLastRecord());
	EXPECT_EQ(me.recordTime, you.recordTime);
	EXPECT_EQ(me.currentWaypoint, you.currentWaypoint);
	EXPECT_EQ(me.waypointStrength, you.waypointStrength);
	EXPECT_EQ(me.lastContact, you.lastContact);
	EXPECT_EQ(me.lastRC, you.lastRC);
	EXPECT_EQ(me.launchPoint.lat, you.launchPoint.lat);
	EXPECT_EQ(me.launchPoint.lon, you.launchPoint.lon);
	EXPECT_EQ(me.lastFix.fix.lat, you.lastFix.fix.lat);
	EXPECT_EQ(me.lastFix.fix.lon, you.lastFix.fix.lon);
	EXPECT_TRUE(you.hasFault("test"));
	EXPECT_EQ(me.getBoatMode(), you.getBoatMode());
	EXPECT_EQ(me.getNavMode(), you.getNavMode());
	EXPECT_EQ(me.getAutoMode(), you.getAutoMode());
	EXPECT_EQ(me.getRCMode(), you.getRCMode());
}

TEST(BoatStateTest, Command) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Start\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"SelfTest\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Disarmed\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Fault\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Navigation\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"ArmedTest\"}", 0, err)));
	EXPECT_EQ(me.commandCnt(), 6);
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.commandCnt(), 5);
	EXPECT_EQ(me.executeCmds(2), 2);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(me.commandCnt(), 3);
	EXPECT_EQ(me.executeCmds(0), 3);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_EQ(me.commandCnt(), 0);
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Start\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"SelfTest\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Disarmed\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Fault\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"Navigation\"}", 0, err)));
	EXPECT_NO_THROW(me.pushCmd("SetMode", json_loads("{\"mode\":\"ArmedTest\"}", 0, err)));
	EXPECT_EQ(me.commandCnt(), 6);
	EXPECT_NO_THROW(me.flushCmds());
	EXPECT_EQ(me.commandCnt(), 0);
}

TEST(BoatStateTest, SetNavMode) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetNavMode", json_loads("{\"mode\":\"Autonomous\"}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.getNavMode(), NavModeEnum::AUTONOMOUS);
}

TEST(BoatStateTest, SetAutoMode) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetAutoMode", json_loads("{\"mode\":\"Return\"}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::RETURN);
}

TEST(BoatStateTest, SetHome) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetHome", json_loads("{\"location\":{\"lat\":5.0,\"lon\":7.5}}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.launchPoint.lat, 5.0);
	EXPECT_EQ(me.launchPoint.lon, 7.5);
	EXPECT_NO_THROW(me.pushCmd("SetHome", json_loads("{\"location\":{\"lon\":7.5}}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 0);
	EXPECT_EQ(me.launchPoint.lat, 5.0);
	EXPECT_EQ(me.launchPoint.lon, 7.5);
}

TEST(BoatStateTest, SetWaypoint) {
	BoatState me;
	json_error_t *err;
	me.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
	EXPECT_EQ(me.waypointList.count(), 7);
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::NONE);
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", json_loads("{\"number\":4}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.waypointList.current(), 4);
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", json_loads("{\"number\":8}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.waypointList.current(), 6);
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", json_loads("{\"number\":-1}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.waypointList.current(), 0);
}

TEST(BoatStateTest, SetWaypointAction) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetWaypointAction", json_loads("{\"action\":\"Return\"}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::RETURN);
	EXPECT_NO_THROW(me.pushCmd("SetWaypointAction", json_loads("{\"action\":\"\"}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 0);
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::RETURN);
}

TEST(BoatStateTest, SetPID) {
	BoatState me;
	json_error_t *err;
	EXPECT_NO_THROW(me.pushCmd("SetPID", json_loads("{\"Kp\":5.0}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(std::get<0>(me.K), 5.0);
	EXPECT_EQ(std::get<1>(me.K), 0.1);
	EXPECT_EQ(std::get<2>(me.K), 0.0);
	EXPECT_NO_THROW(me.pushCmd("SetPID", json_loads("{\"Ki\":0.5,\"Kd\":0.1}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(std::get<0>(me.K), 5.0);
	EXPECT_EQ(std::get<1>(me.K), 0.5);
	EXPECT_EQ(std::get<2>(me.K), 0.1);
	EXPECT_NO_THROW(me.pushCmd("SetPID", json_loads("{\"Kp\":50.0,\"Ki\":5.0,\"Kd\":1.0}", 0, err)));
	EXPECT_EQ(me.executeCmds(1), 1);
	EXPECT_EQ(std::get<0>(me.K), 50.0);
	EXPECT_EQ(std::get<1>(me.K), 5.0);
	EXPECT_EQ(std::get<2>(me.K), 1.0);
}