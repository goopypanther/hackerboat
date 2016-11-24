#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <thread>
#include "boatState.hpp"
#include "boatModes.hpp"
#include "enumdefs.hpp"
#include "test_utilities.hpp"
#include "hal/halTestHarness.hpp"
#include <jansson.h>

class RCModeIdleTest : public ::testing::Test {
	public:
		RCModeIdleTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = RCModeBase::factory(me, RCModeEnum::IDLE);
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
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, &rcchannels, NULL, NULL, NULL);
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
			me.rudder->attach(RUDDER_PORT, RUDDER_PIN);
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
		RCModeBase	 		*mode;
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
		std::map<std::string, int> 	*adcraw;
		std::vector<uint16_t>		*rcchannels;
		
};

TEST_F(RCModeIdleTest, Outputs) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
}

TEST_F(RCModeIdleTest, RudderSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::RUDDER);
}

TEST_F(RCModeIdleTest, CourseSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
}

TEST_F(RCModeIdleTest, FailSafeSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
}

class RCModeRudderTest : public ::testing::Test {
	public:
		RCModeRudderTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = RCModeBase::factory(me, RCModeEnum::RUDDER);
			start = std::chrono::system_clock::now();
			rcchannels->at(RC_MODE_SWITCH) = RC_MIN;
			rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
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
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, &rcchannels, NULL, NULL, NULL);
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
			me.rudder->attach(RUDDER_PORT, RUDDER_PIN);
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
		RCModeBase	 		*mode;
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
		std::map<std::string, int> 	*adcraw;
		std::vector<uint16_t>		*rcchannels;
		
};

TEST_F(RCModeRudderTest, Outputs) {
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 50);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	me.rudder->write(100);
	me.throttle->setThrottle(2);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 100);
	EXPECT_EQ(me.throttle->getThrottle(), 2);
	me.rudder->write(0);
	me.throttle->setThrottle(0);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	me.rudder->write(-50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), -50);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	me.rudder->write(-100);
	me.throttle->setThrottle(2);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), -100);
	EXPECT_EQ(me.throttle->getThrottle(), 2);
	me.rudder->write(0);
	me.throttle->setThrottle(0);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	me.rudder->write(-50);
	me.throttle->setThrottle(-5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), -50);
	EXPECT_EQ(me.throttle->getThrottle(), -5);
	me.rudder->write(-100);
	me.throttle->setThrottle(-2);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), -100);
	EXPECT_EQ(me.throttle->getThrottle(), -2);
	me.rudder->write(0);
	me.throttle->setThrottle(0);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
}

TEST_F(RCModeRudderTest, CourseSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
}

TEST_F(RCModeRudderTest, IdleSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
}

TEST_F(RCModeRudderTest, FailSafeSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
}

class RCModeCourseTest : public ::testing::Test {
	public:
		RCModeCourseTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = RCModeBase::factory(me, RCModeEnum::COURSE);
			start = std::chrono::system_clock::now();
			rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
			rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
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
			harness.accessRC(&rc, NULL, NULL, &rcfailsafe, &rcvalid, &rcchannels, NULL, NULL, NULL);
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
			me.rudder->attach(RUDDER_PORT, RUDDER_PIN);
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
		RCModeBase	 		*mode;
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
		std::map<std::string, int> 	*adcraw;
		std::vector<uint16_t>		*rcchannels;
		
};

TEST_F(RCModeCourseTest, RudderSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::RUDDER);
}

TEST_F(RCModeCourseTest, IdleSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
}

TEST_F(RCModeCourseTest, FailSafeSwitch) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
}

TEST_F(RCModeCourseTest, PIDtestProportional) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;				// Course mode
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;				// RC mode
	rcchannels->at(RC_COURSE_SELECTOR) = RC_MIDDLE_POSN;	// target course 180 degrees
	rcchannels->at(RC_THROTTLE_CH) = RC_MAX;				// max forward throttle
	std::get<0>(me.K) = 10.0;
	std::get<1>(me.K) = 0.0;
	std::get<2>(me.K) = 0.0;
	orientvalue->roll = 0.0;
	orientvalue->pitch = 0.0;
	orientvalue->heading = 181.0;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), -71.2, 0.1));
}
