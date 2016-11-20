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
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::START);
}

class BoatModeSelfTest : public ::testing::Test {
	public:
		BoatModeSelfTest () {
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
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + SELFTEST_DELAY - 15ms);
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
	printf("%s\n", me.getFaultString().c_str());
}