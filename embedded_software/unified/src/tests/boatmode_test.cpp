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

#define TOL 0.000001

TEST(BoatModeTest, BoatModeBase) {
	VLOG(1) << "===Boat Mode Test, Base===";
	BoatState me;
	BoatModeBase *mode = BoatModeBase::factory(me, BoatModeEnum::START);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NONE);
	mode = BoatModeBase::factory(me, BoatModeEnum::SELFTEST);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::SELFTEST);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::START);
	mode = BoatModeBase::factory(me, BoatModeEnum::DISARMED);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::SELFTEST);
	mode = BoatModeBase::factory(me, BoatModeEnum::FAULT);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::FAULT);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::DISARMED);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::FAULT);
	mode = BoatModeBase::factory(me, BoatModeEnum::ARMEDTEST);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::ARMEDTEST);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
	mode = BoatModeBase::factory(me, BoatModeEnum::LOWBATTERY);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::LOWBATTERY);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::ARMEDTEST);
	mode = BoatModeBase::factory(me, BoatModeEnum::NONE);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(me.getBoatMode(), BoatModeEnum::START);
	EXPECT_EQ(me.getBoatMode(), mode->getMode());
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NONE);
}

TEST(BoatModeTest, BoatModeStart) {
	VLOG(1) << "===Boat Mode Test, Start===";
	BoatState me;
	BoatModeBase *mode = BoatModeBase::factory(me, BoatModeEnum::START);
	BoatModeBase *lastMode = mode;
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
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
	VLOG(1) << "===Boat Mode Test, Selftest, Pass===";
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + Conf::get()->selfTestDelay() - 15ms);
	VLOG(1) << "Finished run";
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
}

TEST_F(BoatModeSelfTest, LowBattery) {
	VLOG(1) << "===Boat Mode Test, Selftest, Low Battery===";
	adcraw->at("battery_mon") = 100;
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + Conf::get()->selfTestDelay() - 15ms);
	VLOG(1) << "Finished run";
	VLOG(2) << me;
	VLOG(2) << "Battery input set to: " << adc.getScaledValues().at("battery_mon") 
			<< "/" << adcraw->at("battery_mon");
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::LOWBATTERY);
}

TEST_F(BoatModeSelfTest, LowBatteryRecovery) {
	VLOG(1) << "===Boat Mode Test, Selftest, Low Battery Recovery===";
	adcraw->at("battery_mon") = 100;
	VLOG(2) << "Battery input set to: " << adc.getScaledValues().at("battery_mon") 
			<< "/" << adcraw->at("battery_mon");
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() - 10s) {
			adcraw->at("battery_mon") = 3000;
			VLOG(2) << "Battery input set to: " << adc.getScaledValues().at("battery_mon") 
					<< "/" << adcraw->at("battery_mon");
		}
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + Conf::get()->selfTestDelay() - 15ms);
	VLOG(1) << "Finished run";
	VLOG(2) << me;
	VLOG(2) << "Battery input set to: " << adc.getScaledValues().at("battery_mon") 
			<< "/" << adcraw->at("battery_mon");
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
}

TEST_F(BoatModeSelfTest, ArmFailLow) {
	VLOG(1) << "===Boat Mode Test, Selftest, Arm Fail Low===";
	me.disarmInput.clear();
	VLOG(2) << me;
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + Conf::get()->selfTestDelay() - 15ms);
	VLOG(1) << "Finished run";
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::FAULT);
}

TEST_F(BoatModeSelfTest, ArmFailHigh) {
	VLOG(1) << "===Boat Mode Test, Selftest, Arm Fail High===";
	me.armInput.set();
	VLOG(2) << me;
	while (mode->getMode() == BoatModeEnum::SELFTEST) {
		if (std::chrono::system_clock::now() > start + Conf::get()->selfTestDelay() + 15ms) {
			ADD_FAILURE();
			break;
		}
		health.readHealth();
		mode = mode->execute();
	}
	EXPECT_LT(std::chrono::system_clock::now(), start + Conf::get()->selfTestDelay() - 15ms);
	VLOG(1) << "Finished run";
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
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
				harness.accessRelay(r.second, &drive, &fault);
				fault->init();
				fault->setDir(true);
				fault->clear();
			}
			me.disarmInput.init();
			me.disarmInput.setDir(true);
			me.disarmInput.set();
			me.armInput.init();
			me.armInput.setDir(true);
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
	VLOG(1) << "===Boat Mode Test, Disarmed, Horn===";
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	EXPECT_EQ(me.servoEnable.get(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	#ifdef DISTRIB_IMPLEMENTED
		#pragma message "Compiling Horn test for new power distribution box"
		me.disarmInput.clear();
		me.armInput.set();
	#else
		#pragma message "Compiling Horn test for direct arm/disarm buttons"
		me.disarmInput.set();
		me.armInput.clear();
	#endif /*DISTRIB_IMPLEMENTED*/
	start = std::chrono::system_clock::now();
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	while (mode->getMode() == BoatModeEnum::DISARMED) {
		if (std::chrono::system_clock::now() > start + Conf::get()->hornTime() + 100ms) {
			ADD_FAILURE();
			break;
		}
		EXPECT_EQ(me.relays->get("HORN")->output()->get(), 1);
		mode = mode->execute();
		std::this_thread::sleep_for(50ms);
	}
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(me.relays->get("HORN")->output()->get(), 0);
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
				harness.accessRelay(r.second, &drive, &fault);
				fault->setDir(true);
				fault->init();
				fault->clear();
			}
			me.disarmInput.setDir(true);
			me.disarmInput.init();
			me.armInput.setDir(true);
			me.armInput.init();
			#ifdef DISTRIB_IMPLEMENTED
				#pragma message "Compiling Horn test for new power distribution box"
				me.disarmInput.clear();
				me.armInput.set();
			#else
				#pragma message "Compiling Horn test for direct arm/disarm buttons"
				me.disarmInput.set();
				me.armInput.clear();
			#endif /*DISTRIB_IMPLEMENTED*/
			me.rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
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
	VLOG(1) << "===Boat Mode Test, Nav, Factory===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	VLOG(2) << me;
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last boat mode: " << me.boatModeNames.get(mode->getLastMode());
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< " Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	me.setNavMode(NavModeEnum::FAULT);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	VLOG(2) << me;
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last boat mode: " << me.boatModeNames.get(mode->getLastMode());
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< " Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::FAULT);
	me.setNavMode(NavModeEnum::RC);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	VLOG(2) << me;
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last boat mode: " << me.boatModeNames.get(mode->getLastMode());
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< " Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::RC);
	me.setNavMode(NavModeEnum::AUTONOMOUS);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	VLOG(2) << me;
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last boat mode: " << me.boatModeNames.get(mode->getLastMode());
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< " Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::AUTONOMOUS);
	me.setNavMode(NavModeEnum::NONE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	mynav = (BoatNavigationMode*)mode;
	VLOG(2) << me;
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last boat mode: " << me.boatModeNames.get(mode->getLastMode());
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< " Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
}

TEST_F(BoatModeNavTest, ModeCommandSelfTest) {
	VLOG(1) << "===Boat Mode Test, Nav, Command Self Test===";
	me.setNavMode(NavModeEnum::IDLE);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	me.setBoatMode(BoatModeEnum::SELFTEST);
	mode = mode->execute();
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::SELFTEST);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, ModeCommandDisarm) {
	VLOG(1) << "===Boat Mode Test, Nav, Command Disarm===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	me.setBoatMode(BoatModeEnum::DISARMED);
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	start = std::chrono::system_clock::now();
	mode = mode->execute();
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_GT(std::chrono::system_clock::now(), start + Conf::get()->disarmPulseLen());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, LowBattery) {
	VLOG(1) << "===Boat Mode Test, Nav, Low Battery===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	adcraw->at("battery_mon") = 10;
	health.readHealth();
	VLOG(2) << me;
	VLOG(2) << "Battery input set to: " << adc.getScaledValues().at("battery_mon") 
			<< "/" << adcraw->at("battery_mon");
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::LOWBATTERY);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, NavModeExecute) {
	VLOG(1) << "===Boat Mode Test, Nav Mode Execute===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	mode = mode->execute();
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< ", Last boat mode: " << me.boatModeNames.get(mode->getLastMode())
			<< ", count: " << mode->getCount();
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< ", Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode())
			<< ", count: " << mynav->getNavMode()->getCount();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 1);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 1);
	mode = mode->execute();
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< ", Last boat mode: " << me.boatModeNames.get(mode->getLastMode())
			<< ", count: " << mode->getCount();
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< ", Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode())
			<< ", count: " << mynav->getNavMode()->getCount();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 2);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 2);
	mode = mode->execute();
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< ", Last boat mode: " << me.boatModeNames.get(mode->getLastMode())
			<< ", count: " << mode->getCount();
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< ", Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode())
			<< ", count: " << mynav->getNavMode()->getCount();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 3);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 3);
	mode = mode->execute();
	VLOG(2) << "This boat mode: " << me.boatModeNames.get(mode->getMode())
			<< ", Last boat mode: " << me.boatModeNames.get(mode->getLastMode())
			<< ", count: " << mode->getCount();
	VLOG(2) << "This nav mode: " << me.navModeNames.get(mynav->getNavMode()->getMode())
			<< ", Last nav mode: " << me.navModeNames.get(mynav->getNavMode()->getLastMode())
			<< ", count: " << mynav->getNavMode()->getCount();
	EXPECT_EQ(mode->getMode(), BoatModeEnum::NAVIGATION);
	EXPECT_EQ(mode->getCount(), 4);
	EXPECT_EQ(mynav->getNavMode()->getMode(), NavModeEnum::IDLE);
	EXPECT_EQ(mynav->getNavMode()->getCount(), 4);
}

TEST_F(BoatModeNavTest, Disarm) {
	VLOG(1) << "===Boat Mode Test, Nav, Disarm===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);;
	#ifdef DISTRIB_IMPLEMENTED
		#pragma message "Compiling Disarm test for new power distribution box"
		me.disarmInput.set();
		me.armInput.clear();
	#else
		#pragma message "Compiling Disarm test for direct arm/disarm buttons"
		me.disarmInput.clear();
		me.armInput.set();
	#endif /*DISTRIB_IMPLEMENTED*/
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::DISARMED);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}

TEST_F(BoatModeNavTest, Fault) {
	VLOG(1) << "===Boat Mode Test, Nav, Fault===";
	me.setNavMode(NavModeEnum::IDLE);
	mode = BoatModeBase::factory(me, BoatModeEnum::NAVIGATION);
	//BoatNavigationMode *mynav = (BoatNavigationMode*)mode;
	me.insertFault("Test Fault");
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This mode: " << me.boatModeNames.get(mode->getMode())
			<< " Last mode: " << me.boatModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), BoatModeEnum::FAULT);
	EXPECT_EQ(mode->getLastMode(), BoatModeEnum::NAVIGATION);
}