#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "boatState.hpp"
#include "boatModes.hpp"
#include "enumdefs.hpp"
#include "test_utilities.hpp"
#include "hal/halTestHarness.hpp"
#include <jansson.h>
#include "easylogging++.h"

#define TOL 0.000001

TEST(BoatModeTest, BoatModeBase) {
	BoatState me;
	BoatModeBase *mode = BoatModeBase::factory(me, BoatModeEnum::START);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NONE);
	mode = BoatModeBase::factory(me, BoatModeEnum::SELFTEST);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::SELFTEST);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::START);
	mode = BoatModeBase::factory(me, BoatModeEnum::DISARMED);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::SELFTEST);
	mode = BoatModeBase::factory(me, BoatModeEnum::FAULT);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::FAULT);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::DISARMED);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::FAULT);
	mode = BoatModeBase::factory(me, BoatModeEnum::ARMEDTEST);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
	mode = BoatModeBase::factory(me, BoatModeEnum::LOWBATTERY);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::LOWBATTERY);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::ARMEDTEST);
	mode = BoatModeBase::factory(me, BoatModeEnum::NONE);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NONE);
}

TEST(BoatModeTest, BoatModeStart) {
	BoatState me;
	BoatModeBase *mode = BoatModeBase::factory(me, BoatModeEnum::START);
	BoatModeBase *lastMode = mode;
	mode = mode->execute();
	EXPECT_NE(mode, lastMode);
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::SELFTEST);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
}

class BoatModeSelfTest : public ::testing::Test {
	public:
		BoatModeSelfTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = BoatModeBase::factory(me, BoatModeEnum::SELFTEST);
			start = std::chrono::system_clock::now();
			me.health = &health;
			health.setADCdevice(&adc);
			me.rudder = &rudder;
			me.throttle = &throttle;
			me.rc = &rc;
			me.adc = &adc;
			me.gps = &gps;
			me.orient = &orient;
			me.relays = RelayMap::instance();
			adc.init();
			me.relays->init();
			harness.accessADC(&adc, &adcraw, &adcvalid);
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, NULL, NULL, NULL, NULL);
			harness.accessGPSd(&gps, &fix, NULL);
			harness.accessOrientation(&orient, &orientvalue, &orientvalid);
			for (auto r: *me.relays->getmap()) {
				Pin *drive;
				Pin *fault;
				harness.accessRelay(&(r.second), &drive, &fault);
				fault->setDir(true);
				fault->init();
				fault->clear();
			}
			fix->fixValid = true;
			fix->speed = 0;
			fix->track = 0;
			fix->fix.lat = 48.0;
			fix->fix.lon = -114.0;
			me.lastFix = *fix;
			me.disarmInput.setDir(true);
			me.disarmInput.init();
			me.disarmInput.set();
			me.armInput.setDir(true);
			me.armInput.init();
			me.armInput.clear();
			*adcvalid = true;
			*rcvalid = true;
			*rcfailsafe = false;
			*orientvalid = true;
			adcraw->at("battery_mon") = 3000;
			health.readHealth();
		}
		
		BoatState 			me;
		BoatModeBase 		*mode;
		std::chrono::system_clock::time_point start;
		HealthMonitor 		health;
		Servo 				rudder;
		Throttle 			throttle;
		RCInput 			rc;
		ADCInput 			adc;
		GPSdInput 			gps;
		OrientationInput 	orient;
		HalTestHarness		harness;
		bool 				*adcvalid;
		bool				*rcvalid;
		bool				*rcfailsafe;
		bool				*orientvalid;
		GPSFix				*fix;
		Orientation			*orientvalue;
		std::map<std::string, int> *adcraw;
		
};

TEST_F(BoatModeSelfTest, Pass) {
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
}

TEST_F(BoatModeSelfTest, LowBattery) {
	adcraw->at("battery_mon") = 100;
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::LOWBATTERY);
}

TEST_F(BoatModeSelfTest, LowBatteryRecovery) {
	adcraw->at("battery_mon") = 100;
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY - 10s) {
			adcraw->at("battery_mon") = 3000;
		}
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
}

TEST_F(BoatModeSelfTest, ArmFailLow) {
	me.disarmInput.clear();
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::FAULT);
}

TEST_F(BoatModeSelfTest, ArmFailHigh) {
	me.armInput.set();
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + SELFTEST_DELAY + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::FAULT);
}

class BoatModeDisarmedTest : public ::testing::Test {
	public:
		BoatModeDisarmedTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = BoatModeBase::factory(me, BoatModeEnum::DISARMED);
			start = std::chrono::system_clock::now();
			me.health = &health;
			health.setADCdevice(&adc);
			me.rudder = &rudder;
			me.throttle = &throttle;
			me.rc = &rc;
			me.adc = &adc;
			me.gps = &gps;
			me.orient = &orient;
			me.relays = RelayMap::instance();
			adc.init();
			me.relays->init();
			harness.accessADC(&adc, &adcraw, &adcvalid);
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, NULL, NULL, NULL, NULL);
			harness.accessGPSd(&gps, &fix, NULL);
			harness.accessOrientation(&orient, &orientvalue, &orientvalid);
			for (auto r: *me.relays->getmap()) {
				Pin *drive;
				Pin *fault;
				harness.accessRelay(&(r.second), &drive, &fault);
				fault->setDir(true);
				fault->init();
				fault->clear();
			}
			me.disarmInput.setDir(true);
			me.disarmInput.init();
			me.disarmInput.set();
			me.armInput.setDir(true);
			me.armInput.init();
			me.armInput.clear();
			*adcvalid = true;
			*rcvalid = true;
			*rcfailsafe = false;
			*orientvalid = true;
			adcraw->at("battery_mon") = 3000;
			health.readHealth();
		}
		
		BoatState 			me;
		BoatModeBase 		*mode;
		std::chrono::system_clock::time_point start;
		HealthMonitor 		health;
		Servo 				rudder;
		Throttle 			throttle;
		RCInput 			rc;
		ADCInput 			adc;
		GPSdInput 			gps;
		OrientationInput 	orient;
		HalTestHarness		harness;
		bool 				*adcvalid;
		bool				*rcvalid;
		bool				*rcfailsafe;
		bool				*orientvalid;
		GPSFix				*fix;
		Orientation			*orientvalue;
		std::map<std::string, int> *adcraw;	
};

TEST_F(BoatModeDisarmedTest, Horn) {
	mode = mode->execute();
	EXPECT_EQ(me.servoEnable.get(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	me.disarmInput.clear();
	me.armInput.set();
	start = std::chrono::system_clock::now();
	mode = mode->execute();
	while (mode->getMode() == BoatModeEnum::DISARMED) {
		if (std::chrono::system_clock::now() > start + HORN_TIME + 100ms) {
			ADD_FAILURE();
			break;
		}
		EXPECT_EQ(me.relays->get("HORN").output()->get(), 1);
		mode = mode->execute();
	}
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(me.relays->get("HORN").output()->get(), 0);
}

class BoatModeNavTest : public ::testing::Test {
	public:
		BoatModeNavTest () {
			start = std::chrono::system_clock::now();
			me.health = &health;
			health.setADCdevice(&adc);
			me.rudder = &rudder;
			me.throttle = &throttle;
			me.rc = &rc;
			me.adc = &adc;
			me.gps = &gps;
			me.orient = &orient;
			me.relays = RelayMap::instance();
			adc.init();
			me.relays->init();
			harness.accessADC(&adc, &adcraw, &adcvalid);
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, NULL, NULL, NULL, NULL);
			harness.accessGPSd(&gps, &fix, NULL);
			harness.accessOrientation(&orient, &orientvalue, &orientvalid);
			for (auto r: *me.relays->getmap()) {
				Pin *drive;
				Pin *fault;
				harness.accessRelay(&(r.second), &drive, &fault);
				fault->setDir(true);
				fault->init();
				fault->clear();
			}
			me.disarmInput.setDir(true);
			me.disarmInput.init();
			me.disarmInput.clear();
			me.armInput.setDir(true);
			me.armInput.init();
			me.armInput.set();
			me.rudder->attach(RUDDER_PORT, RUDDER_PIN);
			*adcvalid = true;
			*rcvalid = true;
			*rcfailsafe = false;
			*orientvalid = true;
			adcraw->at("battery_mon") = 3000;
			health.readHealth();
		}
		
		BoatState 			me;
		BoatModeBase 		*mode;
		std::chrono::system_clock::time_point start;
		HealthMonitor 		health;
		Servo 				rudder;
		Throttle 			throttle;
		RCInput 			rc;
		ADCInput 			adc;
		GPSdInput 			gps;
		OrientationInput 	orient;
		HalTestHarness		harness;
		bool 				*adcvalid;
		bool				*rcvalid;
		bool				*rcfailsafe;
		bool				*orientvalid;
		GPSFix				*fix;
		Orientation			*orientvalue;
		std::map<std::string, int> *adcraw;	
};

TEST_F(BoatModeNavTest, Factory) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	me.setNavMode(NavModeEnum::FAULT);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::FAULT);
	me.setNavMode(NavModeEnum::RC);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::RC);
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::AUTONOMOUS);
	me.setNavMode(NavModeEnum::NONE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
}

TEST_F(BoatModeNavTest, ModeCommandSelfTest) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	me.setBoatMode(BoatModeEnum::SELFTEST);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::SELFTEST);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, ModeCommandDisarm) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	me.setBoatMode(BoatModeEnum::DISARMED);
	start = std::chrono::system_clock::now();
	mode = mode->execute();
	EXPECT_GT(std::chrono::system_clock::now(), start + DISARM_PULSE_LEN);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, LowBattery) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	adcraw->at("battery_mon") = 10;
	health.readHealth();
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::LOWBATTERY);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, NavModeExecute) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 1);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 2);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 3);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 4);
}

TEST_F(BoatModeNavTest, Disarm) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	me.armInput.clear();
	me.disarmInput.set();
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, Fault) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	me.insertFault("Test Fault");
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::FAULT);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::FAULT);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}