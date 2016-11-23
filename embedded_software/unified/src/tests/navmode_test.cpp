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

class NavModeIdleTest : public ::testing::Test {
	public:
		NavModeIdleTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = NavModeBase::factory(me, NavModeEnum::IDLE);
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
		NavModeBase 		*mode;
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

TEST_F(NavModeIdleTest, CommandFault) {
	me.setNavMode(NavModeEnum::FAULT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeIdleTest, CommandRC) {
	me.setNavMode(NavModeEnum::RC);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeIdleTest, CommandAuto) {
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

TEST_F(NavModeIdleTest, SwitchRC) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

class NavModeFaultTest : public ::testing::Test {
	public:
		NavModeFaultTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = NavModeBase::factory(me, NavModeEnum::IDLE);
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
		NavModeBase 		*mode;
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

TEST_F(NavModeFaultTest, Entry) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	me.rudder->write(50.0);
	me.throttle->setThrottle(5);
	me.insertFault("Test Fault");
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
}

TEST_F(NavModeFaultTest, Exit) {
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	me.insertFault("Test Fault");
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
	me.clearFaults();
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeFaultTest, CommandIdle) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeFaultTest, CommandRC) {
	me.setNavMode(NavModeEnum::RC);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeFaultTest, CommandAuto) {
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

class NavModeRCTest : public ::testing::Test {
	public:
		NavModeRCTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = NavModeBase::factory(me, NavModeEnum::RC);
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
		NavModeBase 		*mode;
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

TEST_F(NavModeRCTest, AutoSwitchExit) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	rcchannels->at(RC_AUTO_SWITCH) = RC_MIN;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

TEST_F(NavModeRCTest, Fault) {
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	me.insertFault("Test Fault");
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeRCTest, RCModeExecute) {
	rcchannels->at(RC_MODE_SWITCH) = RC_MIDDLE_POSN;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	NavRCMode *myrc = (NavRCMode*)mode;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 1);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 2);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 3);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 4);
}

TEST_F(NavModeRCTest, CommandIdle) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.setNavMode(NavModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeRCTest, CommandFault) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.setNavMode(NavModeEnum::FAULT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeRCTest, CommandAuto) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

class NavModeAutoTest : public ::testing::Test {
	public:
		NavModeAutoTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = NavModeBase::factory(me, NavModeEnum::AUTONOMOUS);
			rcchannels->at(RC_AUTO_SWITCH) = RC_MIN;
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
		NavModeBase 		*mode;
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

TEST_F(NavModeAutoTest, RCSwitchExit) {
	rcchannels->at(RC_AUTO_SWITCH) = RC_MIN;
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	rcchannels->at(RC_AUTO_SWITCH) = RC_MAX;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeAutoTest, Fault) {
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	me.insertFault("Test Fault");
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeAutoTest, CommandIdle) {
	me.setNavMode(NavModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeAutoTest, CommandRC) {
	me.setNavMode(NavModeEnum::RC);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeAutoTest, CommandFault) {
	me.setNavMode(NavModeEnum::FAULT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeAutoTest, AutoModeExecute) {
	NavAutoMode *myauto = (NavAutoMode*)mode;
	rcchannels->at(RC_AUTO_SWITCH) = RC_MIN;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 1);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 2);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 3);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 4);
}
