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

class AutoModeIdleTest : public ::testing::Test {
	public:
		AutoModeIdleTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = AutoModeBase::factory(me, AutoModeEnum::IDLE);
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
		AutoModeBase	 	*mode;
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

TEST_F(AutoModeIdleTest, Outputs) {
	VLOG(1) << "===Auto Mode Idle Test, Outputs===";
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeIdleTest, WaypointTransition) {
	VLOG(1) << "===Auto Mode Idle Test, Waypoint Command===";
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeIdleTest, ReturnTransition) {
	VLOG(1) << "===Auto Mode Idle Test, Return Command===";
	me.setAutoMode(AutoModeEnum::RETURN);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
}

TEST_F(AutoModeIdleTest, AnchorTransition) {
	VLOG(1) << "===Auto Mode Idle Test, Anchor Command===";
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.6906518, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.3799706, 0.0001));
}

class AutoModeWaypointTest : public ::testing::Test {
	public:
		AutoModeWaypointTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = AutoModeBase::factory(me, AutoModeEnum::WAYPOINT);
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
			me.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
			std::get<0>(me.K) = 1.0;
			std::get<1>(me.K) = 0.0;
			std::get<2>(me.K) = 0.0;
		}
		
		BoatState 			me;
		AutoModeBase	 	*mode;
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

TEST_F(AutoModeWaypointTest, CommandIdleTransition) {
	VLOG(1) << "===Auto Mode Waypoint Test, Idle Command===";
	me.setAutoMode(AutoModeEnum::IDLE);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeWaypointTest, CommandReturnTransition) {
	VLOG(1) << "===Auto Mode Waypoint Test, Return Command===";
	me.setAutoMode(AutoModeEnum::RETURN);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
}

TEST_F(AutoModeWaypointTest, CommandAnchorTransition) {
	VLOG(1) << "===Auto Mode Waypoint Test, Anchor Command===";
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.6906518, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.3799706, 0.0001));
}

TEST_F(AutoModeWaypointTest, CourseSelection) {
	VLOG(1) << "===Auto Mode Waypoint Test, Course Selection===";
	me.lastFix.fix.lat = 47.6906518;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.waypointList.setCurrent(0);		// make sure we're on the first waypoint
	orientvalue->updateDeclination(me.lastFix.fix);
	Orientation trueorientation {0, 0, 185.0, false};
	*orientvalue = trueorientation.makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	VLOG(2) << "Target Waypoint: " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	EXPECT_TRUE(orientvalue->isMagnetic());
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), 5, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointAdvance) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint Advance===";
	me.lastFix.fix.lat = 47.59066;		// Near the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.waypointList.setCurrent(0);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	EXPECT_EQ(me.waypointList.current(), 1);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointRepeat) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint Repeat===";
	me.lastFix.fix.lat = 47.59436;	
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::REPEAT);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointReturn) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint Return===";
	me.lastFix.fix.lat = 47.59436;	
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::RETURN);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointAnchor) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint Anchor===";
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::ANCHOR);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.59436, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.37912, 0.0001));
}

TEST_F(AutoModeWaypointTest, WaypointIdle) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint Idle===";
	me.lastFix.fix.lat = 47.59436;	
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::IDLE);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointNone) {
	VLOG(1) << "===Auto Mode Waypoint Test, Waypoint None===";
	me.lastFix.fix.lat = 47.59436;	
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::NONE);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Target Waypoint: " << me.waypointList.current() << ", " << me.waypointList.getWaypoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

class AutoModeReturnTest : public ::testing::Test {
	public:
		AutoModeReturnTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = AutoModeBase::factory(me, AutoModeEnum::RETURN);
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
			me.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
			std::get<0>(me.K) = 1.0;
			std::get<1>(me.K) = 0.0;
			std::get<2>(me.K) = 0.0;
		}
		
		BoatState 			me;
		AutoModeBase	 	*mode;
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

TEST_F(AutoModeReturnTest, CommandIdleTransition) {
	VLOG(1) << "===Auto Mode Return Test, Idle Command===";
	me.setAutoMode(AutoModeEnum::IDLE);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeReturnTest, CommandWaypointTransition) {
	VLOG(1) << "===Auto Mode Return Test, Waypoint Command===";
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeReturnTest, CommandAnchorTransition) {
	VLOG(1) << "===Auto Mode Return Test, Anchor Command===";
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.6906518, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.3799706, 0.0001));
}

TEST_F(AutoModeReturnTest, CourseSelection) {
	VLOG(1) << "===Auto Mode Anchor Test, Course Selection===";
	me.lastFix.fix.lat = 47.6906518;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.launchPoint.lat = 47.5906518;
	me.launchPoint.lon = -122.3799706;
	orientvalue->updateDeclination(me.lastFix.fix);
	Orientation trueorientation {0, 0, 176.0, false};
	*orientvalue = trueorientation.makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	VLOG(2) << "Target: " << me.launchPoint;
	mode = mode->execute();
	VLOG(2) << me;
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), -4, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeReturnTest, ReturnAnchor) {
	VLOG(1) << "===Auto Mode Anchor Test, Anchor===";
	me.lastFix.fix.lat = 47.5906518;	
	me.lastFix.fix.lon = -122.3799706;
	me.launchPoint.lat = 47.5907;
	me.launchPoint.lon = -122.3800;
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

class AutoModeAnchorTest : public ::testing::Test {
	public:
		AutoModeAnchorTest () {
			system("gpsd -n -S 3001 /dev/ttyS4 /dev/ttyACM0");
			mode = AutoModeBase::factory(me, AutoModeEnum::ANCHOR);
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
			me.waypointList.loadKML("/home/debian/hackerboat/embedded_software/unified/test_data/waypoint/test_map_1.kml");
			std::get<0>(me.K) = 1.0;
			std::get<1>(me.K) = 0.0;
			std::get<2>(me.K) = 0.0;
		}
		
		BoatState 			me;
		AutoModeBase	 	*mode;
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

TEST_F(AutoModeAnchorTest, CommandIdleTransition) {
	VLOG(1) << "===Auto Mode Anchor Test, Idle Command===";
	me.setAutoMode(AutoModeEnum::IDLE);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeAnchorTest, CommandReturnTransition) {
	VLOG(1) << "===Auto Mode Anchor Test, Return Command===";
	me.setAutoMode(AutoModeEnum::RETURN);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
}

TEST_F(AutoModeAnchorTest, CommandWaypointTransition) {
	VLOG(1) << "===Auto Mode Anchor Test, Waypoint Command===";
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	VLOG(2) << me;
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeAnchorTest, Forward) {
	VLOG(1) << "===Auto Mode Anchor Test, Forward===";
	me.lastFix.fix.lat = 47.5907;	
	me.lastFix.fix.lon = -122.3800;
	orientvalue->updateDeclination(me.lastFix.fix);
	Orientation trueorientation {0, 0, 190.0, false};
	*orientvalue = trueorientation.makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	mode = mode->execute();
	me.lastFix.fix.lat = 47.5912;
	me.lastFix.fix.lon = -122.3800;
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	std::this_thread::sleep_for(100ms);
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), 10, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeAnchorTest, Reverse) {
	VLOG(1) << "===Auto Mode Anchor Test, Reverse===";
	me.lastFix.fix.lat = 47.5907;	
	me.lastFix.fix.lon = -122.3800;
	orientvalue->updateDeclination(me.lastFix.fix);
	Orientation trueorientation {0, 0, 175.0, false};
	*orientvalue = trueorientation.makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	mode = mode->execute();
	me.lastFix.fix.lat = 47.5902;
	me.lastFix.fix.lon = -122.3800;
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	std::this_thread::sleep_for(100ms);
	VLOG(2) << me;
	VLOG(2) << "Orientation: " << *orientvalue;
	VLOG(2) << "Anchor point: " << ((AutoAnchorMode*)mode)->getAnchorPoint();
	mode = mode->execute();
	VLOG(2) << me;
	VLOG(2) << "This auto mode: " << me.autoModeNames.get(mode->getMode())
			<< ", Last auto mode: " << me.autoModeNames.get(mode->getLastMode());
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), -5, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), -5);
}