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
#include "easylogging++.h"

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
	VLOG(1) << "==RC Mode Idle Test, Outputs===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
}

TEST_F(RCModeIdleTest, RudderSwitch) {
	VLOG(1) << "==RC Mode Idle Test, Rudder Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::RUDDER);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
}

TEST_F(RCModeIdleTest, CourseSwitch) {
	VLOG(1) << "==RC Mode Idle Test, Course Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
}

TEST_F(RCModeIdleTest, FailSafeSwitch) {
	VLOG(1) << "===RC Mode Idle Test, FailSafe===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Failsafe state: " << me.rc->isFailSafe();
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
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
	VLOG(1) << "===RC Mode Rudder Test, Outputs===";
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	
	rcchannels->at(RC_RUDDER_CH) = 1401;
	rcchannels->at(RC_THROTTLE_CH) = RC_MAX;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 50);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	
	rcchannels->at(RC_RUDDER_CH) = RC_MAX;
	rcchannels->at(RC_THROTTLE_CH) = 1350;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 100);
	EXPECT_EQ(me.throttle->getThrottle(), 2);
	
	rcchannels->at(RC_RUDDER_CH) = 991;
	rcchannels->at(RC_THROTTLE_CH) = 991;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	
	rcchannels->at(RC_RUDDER_CH) = 581;
	rcchannels->at(RC_THROTTLE_CH) = RC_MAX;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), -50);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	
	rcchannels->at(RC_RUDDER_CH) = RC_MIN;
	rcchannels->at(RC_THROTTLE_CH) = 1350;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), -100);
	EXPECT_EQ(me.throttle->getThrottle(), 2);
	
	rcchannels->at(RC_RUDDER_CH) = 991;
	rcchannels->at(RC_THROTTLE_CH) = 991;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	
	rcchannels->at(RC_RUDDER_CH) = 581;
	rcchannels->at(RC_THROTTLE_CH) = RC_MIN;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), -50);
	EXPECT_EQ(me.throttle->getThrottle(), -5);
	
	rcchannels->at(RC_RUDDER_CH) = RC_MIN;
	rcchannels->at(RC_THROTTLE_CH) = 650;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), -100);
	EXPECT_EQ(me.throttle->getThrottle(), -2);
	
	rcchannels->at(RC_RUDDER_CH) = 991;
	rcchannels->at(RC_THROTTLE_CH) = 991;
	VLOG(2) << "Writing rudder channel: " << rcchannels->at(RC_RUDDER_CH) << " & throttle channel: " << rcchannels->at(RC_THROTTLE_CH);
	mode = mode->execute();
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
}

TEST_F(RCModeRudderTest, CourseSwitch) {
	VLOG(1) << "==RC Mode Rudder Test, Course Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Executing rudder mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
}

TEST_F(RCModeRudderTest, IdleSwitch) {
	VLOG(1) << "==RC Mode Rudder Test, Idle Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing rudder mode...";
	mode = mode->execute();
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
}

TEST_F(RCModeRudderTest, FailSafeSwitch) {
	VLOG(1) << "===RC Mode Rudder Test, FailSafe===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Failsafe state: " << me.rc->isFailSafe();
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing rudder mode...";
	mode = mode->execute();
	VLOG(2) << "Executing failsafe mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
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
	VLOG(1) << "==RC Mode Course Test, Rudder Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Executing course mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::RUDDER);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
}

TEST_F(RCModeCourseTest, IdleSwitch) {
	VLOG(1) << "==RC Mode Course Test, Idle Switch===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing course mode...";
	mode = mode->execute();
	VLOG(2) << "Executing idle mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::IDLE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
}

TEST_F(RCModeCourseTest, FailSafeSwitch) {
	VLOG(1) << "===RC Mode Course Test, FailSafe===";
	rcchannels->at(RC_MODE_SWITCH) = RC_MAX;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	*rcfailsafe = true;
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Failsafe state: " << me.rc->isFailSafe();
	VLOG(2) << "Writing rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
	VLOG(2) << "Executing course mode...";
	mode = mode->execute();
	VLOG(2) << "Executing failsafe mode...";
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), RCModeEnum::FAILSAFE);
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
}

TEST_F(RCModeCourseTest, PIDtestProportional) {
	VLOG(1) << "===RC Mode Course Test, PID (proportional) Test===";
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
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "RC mode switch: " << BoatState::rcModeNames.get(me.rc->getMode());
	VLOG(2) << "Auto switch: " << me.rc->getChannel(RC_AUTO_SWITCH);
	VLOG(2) << "Target course: " << me.rc->getCourse() << " Current course: " << orientvalue->heading;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), RCModeEnum::COURSE);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), 10.0, 0.1));
	VLOG(2) << "Current RC mode: " << BoatState::rcModeNames.get(mode->getMode());
	VLOG(2) << "Output of rudder: " << me.rudder->read() << " & throttle: " << me.throttle->getThrottle();
}
