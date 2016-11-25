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
	me.rudder->write(50);
	me.throttle->setThrottle(5);
	mode = mode->execute();
	EXPECT_EQ(me.rudder->read(), 0);
	EXPECT_EQ(me.throttle->getThrottle(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeIdleTest, WaypointTransition) {
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeIdleTest, ReturnTransition) {
	me.setAutoMode(AutoModeEnum::RETURN);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
}

TEST_F(AutoModeIdleTest, AnchorTransition) {
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
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
	me.setAutoMode(AutoModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeWaypointTest, CommandReturnTransition) {
	me.setAutoMode(AutoModeEnum::RETURN);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
}

TEST_F(AutoModeWaypointTest, CommandAnchorTransition) {
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.6906518, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.3799706, 0.0001));
}

TEST_F(AutoModeWaypointTest, CourseSelection) {
	me.lastFix.fix.lat = 47.6906518;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.waypointList.setCurrent(0);		// make sure we're on the first waypoint
	orientvalue->updateDeclination(me.lastFix.fix);
	orientvalue->roll = 0.0;
	orientvalue->pitch = 0.0;
	orientvalue->heading = 180.0;
	orientvalue->makeTrue();
	orientvalue->heading = 181.0;
	orientvalue->makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	mode = mode->execute();
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), 16.8, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointAdvance) {
	me.lastFix.fix.lat = 47.59066;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.waypointList.setCurrent(0);		// make sure we're on the first waypoint
	mode = mode->execute();
	EXPECT_EQ(me.waypointList.current(), 1);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointRepeat) {
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::REPEAT);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	mode = mode->execute();
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointReturn) {
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::RETURN);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointAnchor) {
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::ANCHOR);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
	mode = mode->execute();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.59436, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.37912, 0.0001));
}

TEST_F(AutoModeWaypointTest, WaypointIdle) {
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::IDLE);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeWaypointTest, WaypointNone) {
	me.lastFix.fix.lat = 47.59436;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.37912;
	me.waypointList.setAction(WaypointActionEnum::NONE);
	me.waypointList.setCurrent(6);		// make sure we're on the first waypoint
	mode = mode->execute();
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
	me.setAutoMode(AutoModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeReturnTest, CommandReturnTransition) {
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeReturnTest, CommandAnchorTransition) {
	me.setAutoMode(AutoModeEnum::ANCHOR);
	me.lastFix.fix.lat = 47.6906518;	
	me.lastFix.fix.lon = -122.3799706;
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::ANCHOR);
	mode = mode->execute();
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lat, 47.6906518, 0.0001));
	EXPECT_TRUE(toleranceEquals(((AutoAnchorMode*)mode)->getAnchorPoint().lon, -122.3799706, 0.0001));
}

TEST_F(AutoModeReturnTest, CourseSelection) {
	me.lastFix.fix.lat = 47.6906518;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.launchPoint.lat = 47.5906518;
	me.launchPoint.lon = -122.3799706;
	orientvalue->updateDeclination(me.lastFix.fix);
	orientvalue->roll = 0.0;
	orientvalue->pitch = 0.0;
	orientvalue->heading = 180.0;
	orientvalue->makeTrue();
	orientvalue->heading = 181.0;
	orientvalue->makeMag();				// the intent here is to get the magnetic bearing equivalent to 1 degree off the true bearing to the first waypoint
	mode = mode->execute();
	EXPECT_EQ(me.waypointList.current(), 0);
	EXPECT_EQ(mode->getMode(), AutoModeEnum::RETURN);
	EXPECT_TRUE(toleranceEquals(me.rudder->read(), 16.8, 0.1));
	EXPECT_EQ(me.throttle->getThrottle(), 5);
}

TEST_F(AutoModeReturnTest, ReturnAnchor) {
	me.lastFix.fix.lat = 47.5906518;	// this location is 0.1 degrees (about six nm) due north of the first waypoint
	me.lastFix.fix.lon = -122.3799706;
	me.launchPoint.lat = 47.5907;
	me.launchPoint.lon = -122.3800;
	mode = mode->execute();
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
	me.setAutoMode(AutoModeEnum::IDLE);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::IDLE);
}

TEST_F(AutoModeAnchorTest, CommandReturnTransition) {
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

TEST_F(AutoModeAnchorTest, CommandWaypointTransition) {
	me.setAutoMode(AutoModeEnum::WAYPOINT);
	mode = mode->execute();
	EXPECT_EQ(mode->getMode(), AutoModeEnum::WAYPOINT);
}

