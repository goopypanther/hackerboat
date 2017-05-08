#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "boatState.hpp"
#include "boatModes.hpp"
#include "enumdefs.hpp"
#include "test_utilities.hpp"
#include "hal/halTestHarness.hpp"
#include "easylogging++.h"
#include "configuration.hpp"

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
				harness.accessRelay(r.second, &drive, &fault);
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
			me.rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
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
			adcraw->at(Conf::get()->batmonName()) = 3000;
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
	VLOG(1) << "===Nav Mode Idle Test, Command Fault===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::FAULT);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeIdleTest, CommandRC) {
	VLOG(1) << "===Nav Mode Idle Test, Command RC===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::RC);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeIdleTest, CommandAuto) {
	VLOG(1) << "===Nav Mode Idle Test, Command Auto===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

TEST_F(NavModeIdleTest, SwitchRC) {
	VLOG(1) << "===Nav Mode Idle Test, Switch RC===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
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
				harness.accessRelay(r.second, &drive, &fault);
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
			me.rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
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
			adcraw->at(Conf::get()->batmonName()) = 3000;
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
	VLOG(1) << "===Nav Mode Fault Test, Entry===";
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	me.rudder->write(50.0);
	me.throttle->setThrottle(5);
	VLOG(2) << "Rudder setting: " << me.rudder->read() << " throttle setting: " << me.throttle->getThrottle();
	me.insertFault("Test Fault");
	VLOG(2) << "Fault string: " << me.getFaultString();
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "Rudder setting: " << me.rudder->read() << " throttle setting: " << me.throttle->getThrottle();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
}

TEST_F(NavModeFaultTest, Exit) {
	VLOG(1) << "===Nav Mode Fault Test, Exit===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	me.insertFault("Test Fault");
	VLOG(2) << "Fault string: " << me.getFaultString();
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
	VLOG(2) << "Clearing faults...";
	me.clearFaults();
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
}

TEST_F(NavModeFaultTest, CommandIdle) {
	VLOG(1) << "===Nav Mode Fault Test, Command Idle===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::IDLE);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeFaultTest, CommandRC) {
	VLOG(1) << "===Nav Mode Fault Test, Command RC===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::RC);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeFaultTest, CommandAuto) {
	VLOG(1) << "===Nav Mode Fault Test, Command Auto===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
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
				harness.accessRelay(r.second, &drive, &fault);
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
			me.rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
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
			adcraw->at(Conf::get()->batmonName()) = 3000;
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
	VLOG(1) << "===Nav Mode RC Test, Switch Auto===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("min");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

TEST_F(NavModeRCTest, Fault) {
	VLOG(1) << "===Nav Mode RC Test, Fault===";
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	me.insertFault("Test Fault");
	VLOG(2) << "Fault string: " << me.getFaultString();
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeRCTest, RCModeExecute) {
	VLOG(1) << "===Nav Mode RC Test, Execute===";
	rcchannels->at(Conf::get()->RCchannelMap().at("mode")) = Conf::get()->RClimits().at("middlePosn");
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(2) << "Setting RC mode switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("mode"));
	NavRCMode *myrc = (NavRCMode*)mode;
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New rc mode set to " << me.rcModeNames.get(myrc->getRCMode()->getMode());
	VLOG(2) << "RC mode count is " << myrc->getRCMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 1);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New rc mode set to " << me.rcModeNames.get(myrc->getRCMode()->getMode());
	VLOG(2) << "RC mode count is " << myrc->getRCMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 2);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New rc mode set to " << me.rcModeNames.get(myrc->getRCMode()->getMode());
	VLOG(2) << "RC mode count is " << myrc->getRCMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 3);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New rc mode set to " << me.rcModeNames.get(myrc->getRCMode()->getMode());
	VLOG(2) << "RC mode count is " << myrc->getRCMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(myrc->getRCMode()->getMode(), RCModeEnum::IDLE);
	EXPECT_EQ(myrc->getRCMode()->getCount(), 4);
}

TEST_F(NavModeRCTest, CommandIdle) {
	VLOG(1) << "===Nav Mode RC Test, Command Idle===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	me.setNavMode(NavModeEnum::IDLE);
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeRCTest, CommandFault) {
	VLOG(1) << "===Nav RC Fault Test, Command Fault===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	me.setNavMode(NavModeEnum::FAULT);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeRCTest, CommandAuto) {
	VLOG(1) << "===Nav RC Fault Test, Command Auto===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
}

class NavModeAutoTest : public ::testing::Test {
	public:
		NavModeAutoTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = NavModeBase::factory(me, NavModeEnum::AUTONOMOUS);
			rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("min");
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
				harness.accessRelay(r.second, &drive, &fault);
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
			me.rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
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
			adcraw->at(Conf::get()->batmonName()) = 3000;
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
	VLOG(1) << "===Nav Mode Auto Test, Switch RC===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("min");
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("max");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeAutoTest, Fault) {
	VLOG(1) << "===Nav Mode Auto Test, Fault===";
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	me.insertFault("Test Fault");
	VLOG(2) << "Fault string: " << me.getFaultString();
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeAutoTest, CommandIdle) {
	VLOG(1) << "===Nav Mode Auto Test, Command Idle===";
	VLOG(2) << "Starting nav mode set to " << me.navModeNames.get(mode->getMode());
	me.setNavMode(NavModeEnum::IDLE);
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::IDLE);
}

TEST_F(NavModeAutoTest, CommandRC) {
	VLOG(1) << "===Nav Mode Auto Test, Command RC===";
	me.setNavMode(NavModeEnum::RC);
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::RC);
}

TEST_F(NavModeAutoTest, CommandFault) {
	VLOG(1) << "===Nav Mode Auto Test, Command Fault===";
	me.setNavMode(NavModeEnum::FAULT);
	VLOG(2) << "Setting nav mode to " << me.navModeNames.get(me.getNavMode());
	mode = mode->execute();
	VLOG(2) << "Nav mode set to " << me.navModeNames.get(mode->getMode());
	EXPECT_EQ(mode->getMode(), NavModeEnum::FAULT);
}

TEST_F(NavModeAutoTest, AutoModeExecute) {
	VLOG(1) << "===Nav Mode Auto Test, Execute===";
	NavAutoMode *myauto = (NavAutoMode*)mode;
	rcchannels->at(Conf::get()->RCchannelMap().at("auto")) = Conf::get()->RClimits().at("min");
	VLOG(2) << "Setting RC/Auto switch to " << rcchannels->at(Conf::get()->RCchannelMap().at("auto"));
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New auto mode set to " << me.autoModeNames.get(myauto->getAutoMode()->getMode());
	VLOG(2) << "Auto mode count is " << myauto->getAutoMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 1);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New auto mode set to " << me.autoModeNames.get(myauto->getAutoMode()->getMode());
	VLOG(2) << "Auto mode count is " << myauto->getAutoMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 2);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New auto mode set to " << me.autoModeNames.get(myauto->getAutoMode()->getMode());
	VLOG(2) << "Auto mode count is " << myauto->getAutoMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 3);
	VLOG(1) << "Executing nav mode...";
	mode = mode->execute();
	VLOG(2) << "New nav mode set to " << me.navModeNames.get(mode->getMode());
	VLOG(2) << "New auto mode set to " << me.autoModeNames.get(myauto->getAutoMode()->getMode());
	VLOG(2) << "Auto mode count is " << myauto->getAutoMode()->getCount();
	VLOG(2) << "Nav mode count is " << mode->getCount();
	EXPECT_EQ(mode->getMode(), NavModeEnum::AUTONOMOUS);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(myauto->getAutoMode()->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(myauto->getAutoMode()->getCount(), 4);
}
