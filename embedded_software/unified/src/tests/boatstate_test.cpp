#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "boatState.hpp"
#include "enumdefs.hpp"
#include "test_utilities.hpp"
#include "rapidjson/rapidjson.h"
#include "easylogging++.h"

#define TOL 0.000001

using namespace rapidjson;

TEST(BoatStateTest, FaultString) {
	VLOG(1) << "===Boat State Test, Fault String===";
	BoatState me;
	EXPECT_EQ(me.faultCount(), 0);
	EXPECT_TRUE(me.insertFault("one"));
	VLOG(2) << "Fault string: " << me.getFaultString();
	EXPECT_EQ(me.faultCount(), 1);
	EXPECT_TRUE(me.insertFault("one"));
	VLOG(2) << "Fault string: " << me.getFaultString();
	EXPECT_EQ(me.faultCount(), 1);
	EXPECT_TRUE(me.insertFault("two"));
	VLOG(2) << "Fault string: " << me.getFaultString();
	EXPECT_EQ(me.faultCount(), 2);
	EXPECT_TRUE(me.hasFault("one"));
	EXPECT_TRUE(me.hasFault("two"));
	EXPECT_FALSE(me.hasFault("three"));
	EXPECT_TRUE(me.removeFault("two"));
	VLOG(2) << "Fault string: " << me.getFaultString();
	EXPECT_FALSE(me.hasFault("two"));
	EXPECT_EQ(me.faultCount(), 1);
	me.clearFaults();
	VLOG(2) << "Fault string: " << me.getFaultString();
	EXPECT_FALSE(me.hasFault("one"));
	EXPECT_EQ(me.faultCount(), 0);
}

TEST(BoatStateTest, BoatMode) {
	VLOG(1) << "===Boat State Test, Boat Mode===";
	BoatState me;
	EXPECT_FALSE(me.setBoatMode(""));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_TRUE(me.setBoatMode("Start"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_TRUE(me.setBoatMode("SelfTest"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::SELFTEST);
	EXPECT_TRUE(me.setBoatMode("Disarmed"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_TRUE(me.setBoatMode("Fault"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::FAULT);
	EXPECT_TRUE(me.setBoatMode("Navigation"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NAVIGATION);
	EXPECT_TRUE(me.setBoatMode("ArmedTest"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_TRUE(me.setBoatMode("None"));
	VLOG(2) << "Boat mode: " << me.boatModeNames.get(me.getBoatMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NONE);
}

TEST(BoatStateTest, NavMode) {
	VLOG(1) << "===Boat State Test, Nav Mode===";
	BoatState me;
	EXPECT_FALSE(me.setNavMode(""));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_TRUE(me.setNavMode("Idle"));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_EQ(me.getNavMode(), NavModeEnum::IDLE);
	EXPECT_TRUE(me.setNavMode("Fault"));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_EQ(me.getNavMode(), NavModeEnum::FAULT);
	EXPECT_TRUE(me.setNavMode("RC"));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_EQ(me.getNavMode(), NavModeEnum::RC);
	EXPECT_TRUE(me.setNavMode("Autonomous"));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_EQ(me.getNavMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_TRUE(me.setNavMode("None"));
	VLOG(2) << "Nav mode: " << me.navModeNames.get(me.getNavMode());
	EXPECT_EQ(me.getNavMode(), NavModeEnum::NONE);
}

TEST(BoatStateTest, AutoMode) {
	VLOG(1) << "===Boat State Test, Auto Mode===";
	BoatState me;
	EXPECT_FALSE(me.setAutoMode(""));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_TRUE(me.setAutoMode("Idle"));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::IDLE);
	EXPECT_TRUE(me.setAutoMode("Waypoint"));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::WAYPOINT);
	EXPECT_TRUE(me.setAutoMode("Return"));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::RETURN);
	EXPECT_TRUE(me.setAutoMode("Anchor"));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::ANCHOR);
	EXPECT_TRUE(me.setAutoMode("None"));
	VLOG(2) << "Auto mode: " << me.autoModeNames.get(me.getAutoMode());
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::NONE);
}

TEST(BoatStateTest, RCMode) {
	VLOG(1) << "===Boat State Test, RC Mode===";
	BoatState me;
	EXPECT_FALSE(me.setRCmode(""));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_TRUE(me.setRCmode("Idle"));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_EQ(me.getRCMode(), RCModeEnum::IDLE);
	EXPECT_TRUE(me.setRCmode("Rudder"));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_EQ(me.getRCMode(), RCModeEnum::RUDDER);
	EXPECT_TRUE(me.setRCmode("Course"));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_EQ(me.getRCMode(), RCModeEnum::COURSE);
	EXPECT_TRUE(me.setRCmode("Failsafe"));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_EQ(me.getRCMode(), RCModeEnum::FAILSAFE);
	EXPECT_TRUE(me.setRCmode("None"));
	VLOG(2) << "RC mode: " << me.rcModeNames.get(me.getRCMode());
	EXPECT_EQ(me.getRCMode(), RCModeEnum::NONE);
}

TEST(BoatStateTest, JSON) {
	VLOG(1) << "===Boat State Test, JSON===";
	BoatState me, you;
	GPSFix testfix;
	testfix.fix = Location(5.0, 5.0);
	me.relays = RelayMap::instance();
	me.recordTime = std::chrono::system_clock::now();
	//me.currentWaypoint = 4;
	//me.waypointStrength = 20.0;
	me.lastContact = std::chrono::system_clock::now();
	me.lastRC = std::chrono::system_clock::now();
	me.launchPoint = Location(10.0, 10.0);
	me.lastFix = testfix;
	me.insertFault("test");
	me.setBoatMode("Disarmed");
	me.setNavMode("Fault");
	me.setAutoMode("Waypoint");
	me.setRCmode("Rudder");
	Value packed;
	ASSERT_NO_THROW(packed = me.pack());
	ASSERT_TRUE(packed.IsObject());
	VLOG(2) << "Packed JSON: " << packed;
	EXPECT_TRUE(you.parse(packed));
	VLOG(2) << "Parsed JSON: " << you;
	EXPECT_EQ(floor<milliseconds>(me.recordTime), you.recordTime);
	//EXPECT_EQ(me.currentWaypoint, you.currentWaypoint);
	//EXPECT_EQ(me.waypointStrength, you.waypointStrength);
	EXPECT_EQ(floor<milliseconds>(me.lastContact), you.lastContact);
	EXPECT_EQ(floor<milliseconds>(me.lastRC), you.lastRC);
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
	VLOG(1) << "===Boat State Test, Command List===";
	BoatState me;
	Document start, selftest, disarmed, fault, navigation, armedtest;
	Value default_val;
	start.Parse("{\"mode\":\"Start\"}");
	EXPECT_TRUE(start.IsObject());
	EXPECT_TRUE(start.HasMember("mode"));
	selftest.Parse("{\"mode\":\"SelfTest\"}");
	EXPECT_TRUE(selftest.IsObject());
	EXPECT_TRUE(selftest.HasMember("mode"));
	disarmed.Parse("{\"mode\":\"Disarmed\"}");
	EXPECT_TRUE(disarmed.IsObject());
	EXPECT_TRUE(disarmed.HasMember("mode"));
	fault.Parse("{\"mode\":\"Fault\"}");
	EXPECT_TRUE(fault.IsObject());
	EXPECT_TRUE(fault.HasMember("mode"));
	navigation.Parse("{\"mode\":\"Navigation\"}");
	EXPECT_TRUE(navigation.IsObject());
	EXPECT_TRUE(navigation.HasMember("mode"));
	armedtest.Parse("{\"mode\":\"ArmedTest\"}");
	EXPECT_TRUE(armedtest.IsObject());
	EXPECT_TRUE(armedtest.HasMember("mode"));

	EXPECT_NO_THROW(me.pushCmd("SetMode", start));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", selftest));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", disarmed));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", fault));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", navigation));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", armedtest));
	VLOG(2) << me;
	EXPECT_EQ(me.commandCnt(), 6);
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.commandCnt(), 5);
	EXPECT_EQ(me.executeCmds(2), 2);
	VLOG(2) << me;
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(me.commandCnt(), 3);
	EXPECT_EQ(me.executeCmds(0), 3);
	VLOG(2) << me;
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_EQ(me.commandCnt(), 0);
	EXPECT_NO_THROW(me.pushCmd("SetMode", start));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", selftest));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", disarmed));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", fault));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", navigation));
	VLOG(2) << me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", armedtest));
	VLOG(2) << me;
	EXPECT_EQ(me.commandCnt(), 6);
	EXPECT_NO_THROW(me.flushCmds());
	VLOG(2) << me;
	EXPECT_EQ(me.commandCnt(), 0);
}

TEST(BoatStateTest, SetNavMode) {
	VLOG(1) << "===Boat State Test, Command Nav Mode===";
	BoatState me;
	Document d;
	Value default_val;
	d.Parse("{\"mode\":\"Autonomous\"}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("mode"));
	EXPECT_NO_THROW(me.pushCmd("SetNavMode", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(me.getNavMode(), NavModeEnum::AUTONOMOUS);
}

TEST(BoatStateTest, SetAutoMode) {
	VLOG(1) << "===Boat State Test, Command Auto Mode===";
	BoatState me;
	Document d;
	d.Parse("{\"mode\":\"Return\"}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("mode"));
	EXPECT_NO_THROW(me.pushCmd("SetAutoMode", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(me.getAutoMode(), AutoModeEnum::RETURN);
}

TEST(BoatStateTest, SetHome) {
	VLOG(1) << "===Boat State Test, Command Set Home===";
	BoatState me;
	Document d;
	Value default_val;
	d.Parse("{\"location\":{\"lat\":5.0,\"lon\":7.5}}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("location"));
	EXPECT_NO_THROW(me.pushCmd("SetHome", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(me.launchPoint.lat, 5.0);
	EXPECT_EQ(me.launchPoint.lon, 7.5);
	d.Parse("{\"location\":{\"lon\":7.5}}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("location"));
	EXPECT_NO_THROW(me.pushCmd("SetHome", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 0);
	VLOG(2) << me;
	EXPECT_EQ(me.launchPoint.lat, 5.0);
	EXPECT_EQ(me.launchPoint.lon, 7.5);
}

TEST(BoatStateTest, SetWaypoint) {
	VLOG(1) << "===Boat State Test, Command Set Waypoint===";
	BoatState me;
	Document d;
	Value default_val;
	me.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
	VLOG(2) << "Count: " << me.waypointList.count()
			<< ", current: " << me.waypointList.current()
			<< ", action: "
			<< me.waypointList.actionNames.get(me.waypointList.getAction());
	EXPECT_EQ(me.waypointList.count(), 7);
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::NONE);
	d.Parse("{\"number\":4}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("number"));
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", d));
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << "Count: " << me.waypointList.count() << ", current: " << me.waypointList.current();
	EXPECT_EQ(me.waypointList.current(), 4);
	d.Parse("{\"number\":8}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("number"));
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", d));
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << "Count: " << me.waypointList.count()
			<< ", current: " << me.waypointList.current();
	EXPECT_EQ(me.waypointList.current(), 6);
	d.Parse("{\"number\":-1}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("number"));
	EXPECT_NO_THROW(me.pushCmd("SetWaypoint", d));
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << "Count: " << me.waypointList.count() << ", current: " << me.waypointList.current();
	EXPECT_EQ(me.waypointList.current(), 0);
}

TEST(BoatStateTest, SetWaypointAction) {
	VLOG(1) << "===Boat State Test, Command Set Waypoint Action===";
	BoatState me;
	Document d;
	Value default_val;
	d.Parse("{\"action\":\"Return\"}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("action"));
	EXPECT_NO_THROW(me.pushCmd("SetWaypointAction", d));
	VLOG(2) << "Action: " << me.waypointList.actionNames.get(me.waypointList.getAction());
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << "Action: " << me.waypointList.actionNames.get(me.waypointList.getAction());
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::RETURN);
	d.Parse("{\"action\":\"\"}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("action"));
	EXPECT_NO_THROW(me.pushCmd("SetWaypointAction", d));
	EXPECT_EQ(me.executeCmds(1), 0);
	VLOG(2) << "Action: " << me.waypointList.actionNames.get(me.waypointList.getAction());
	EXPECT_EQ(me.waypointList.getAction(), WaypointActionEnum::RETURN);
}

TEST(BoatStateTest, SetPID) {
	VLOG(1) << "===Boat State Test, Command Set PID===";
	BoatState me;
	Document d;
	Value default_val;
	d.Parse("{\"Kp\":5.0}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("Kp"));
	EXPECT_NO_THROW(me.pushCmd("SetPID", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(std::get<0>(me.K), 5.0);
	EXPECT_EQ(std::get<1>(me.K), 0.1);
	EXPECT_EQ(std::get<2>(me.K), 0.0);
	d.Parse("{\"Ki\":0.5,\"Kd\":0.1}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("Ki"));
	ASSERT_TRUE(d.HasMember("Kd"));
	EXPECT_NO_THROW(me.pushCmd("SetPID", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(std::get<0>(me.K), 5.0);
	EXPECT_EQ(std::get<1>(me.K), 0.5);
	EXPECT_EQ(std::get<2>(me.K), 0.1);
	d.Parse("{\"Kp\":50.0,\"Ki\":5.0,\"Kd\":1.0}");
	ASSERT_TRUE(d.IsObject());
	ASSERT_TRUE(d.HasMember("Kp"));
	ASSERT_TRUE(d.HasMember("Ki"));
	ASSERT_TRUE(d.HasMember("Kd"));
	EXPECT_NO_THROW(me.pushCmd("SetPID", d));
	VLOG(2) << me;
	EXPECT_EQ(me.executeCmds(1), 1);
	VLOG(2) << me;
	EXPECT_EQ(std::get<0>(me.K), 50.0);
	EXPECT_EQ(std::get<1>(me.K), 5.0);
	EXPECT_EQ(std::get<2>(me.K), 1.0);
}
