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

TEST(BoatStateTest, Command) {
	BoatState me;
	EXPECT_NO_THROW(me.pushCmd("SetMode", "{mode:\"Start\"}")
}

