#include <stdexcept>
#include <gtest/gtest.h>
#include <cmath>
#include "location.hpp"
#include "orientation.hpp"
#include "test_utilities.hpp"
#include "easylogging++.h"
#include "rapidjson/rapidjson.h"

using namespace rapidjson;

#define TOL (0.00001)	// Tolerance for floating point comparisons

TEST (Orientation, Creation) {
	VLOG(1) << "===Orientation Creation Test===";
	Orientation good { 20.0, -15.0, 60.0 };
	Orientation bad;
	Orientation bad_pitch { 20.0, NAN, 60.0 };
	Orientation bad_roll { NAN, -15.0, 60.0 };
	Orientation bad_heading { 20.0, -15.0, NAN };
	
	EXPECT_TRUE(good.isValid());
	EXPECT_FALSE(bad.isValid());
	EXPECT_FALSE(bad_pitch.isValid());
	bad_pitch.pitch = 20.0;
	EXPECT_TRUE(bad_pitch.isValid());
	EXPECT_FALSE(bad_roll.isValid());
	bad_roll.roll = -15.0;
	EXPECT_TRUE(bad_roll.isValid());
	EXPECT_FALSE(bad_heading.isValid());
	bad_heading.heading = 60.0;
	EXPECT_TRUE(bad_heading.isValid());
}

TEST (Orientation, JSON) {
	VLOG(1) << "===Orientation JSON Test===";
	Orientation u { 20.0, -15.0, 60.0 };
	Orientation v;
	Value orient;
	
	orient = u.pack();
	VLOG_IF(orient, 2) << "Output JSON: " << orient;
	v.parse(orient);
	VLOG_IF(v.pack(), 2) << "Parsed JSON: " << orient;
	EXPECT_TRUE(toleranceEquals(u.pitch, v.pitch, TOL));
	EXPECT_TRUE(toleranceEquals(u.roll, v.roll, TOL));
	EXPECT_TRUE(toleranceEquals(u.heading, v.heading, TOL));
}

TEST (Orientation, Normalize) {
	VLOG(1) << "===Orientation Normalize Test===";
	Orientation x { 220.0, -185.0, -60.0 };
	Orientation y { -220.0, 185.0, 460.0 };
	VLOG(2) << "Raw x " << x.pack();
	VLOG(2) << "Raw y " << y.pack();
	EXPECT_TRUE(x.normalize());
	EXPECT_TRUE(y.normalize());
	VLOG(2) << "Normalized x " << x.pack();
	VLOG(2) << "Normalized y " << y.pack();
	EXPECT_TRUE(toleranceEquals(x.roll, -140.0, TOL));
	EXPECT_TRUE(toleranceEquals(y.roll, 140.0, TOL));
	EXPECT_TRUE(toleranceEquals(x.pitch, 175.0, TOL));
	EXPECT_TRUE(toleranceEquals(y.pitch, -175.0, TOL));
	EXPECT_TRUE(toleranceEquals(x.heading, 300.0, TOL));
	EXPECT_TRUE(toleranceEquals(y.heading, 100.0, TOL));
}

TEST (Orientation, HeadingError) {
	VLOG(1) << "===Orientation Heading Error Test===";
	Orientation x { 20.0, -15.0, 10.0 };
	VLOG(2) << "Orientation x " << x.pack();
	VLOG(2) << "Heading error between x & 350 deg: " << x.headingError(350.0);
	EXPECT_TRUE(toleranceEquals(x.headingError(350.0), -20.0, TOL));
	VLOG(2) << "Heading error between x & 30 deg: " << x.headingError(30.0);
	EXPECT_TRUE(toleranceEquals(x.headingError(30.0), 20.0, TOL));
	VLOG(2) << "Heading error between x & 180 deg: " << x.headingError(180.0);
	EXPECT_TRUE(toleranceEquals(x.headingError(180.0), 170.0, TOL));
	VLOG(2) << "Heading error between x & 370 deg: " << x.headingError(370.0);
	EXPECT_TRUE(toleranceEquals(x.headingError(370.0), 0.0, TOL));
	VLOG(2) << "Heading error between x & -20 deg: " << x.headingError(-20.0);
	EXPECT_TRUE(toleranceEquals(x.headingError(-10.0), -20.0, TOL));
	Orientation y { 20.0, -15.0, 180.0 };
	VLOG(2) << "Orientation y " << y.pack();
	EXPECT_TRUE(toleranceEquals(y.headingError(181.0), 1.0, TOL));
}

TEST (Orientation, MakeTrue) {
	VLOG(1) << "===Orientation Declination (Make True) Test===";
	// location and orientation values 
	double expectedDeclination = 15.95;
	Orientation x { 20.0, -15.0, 10.0 };
	Location y { 47.82167, -122.33639 };
	Orientation z { 0.0, 0.0, 240.0 };
	VLOG(2) << "Orientation x " << x.pack();
	VLOG(2) << "Location y " << y.pack();
	VLOG(2) << "Orientation z " << z.pack();
	VLOG(2) << "Expected Declination " << expectedDeclination;
	
	EXPECT_TRUE(x.isMagnetic());
	EXPECT_TRUE(x.updateDeclination(y));
	VLOG(2) << "Declination is " << x.getDeclination() << ", " << z.getDeclination();
	EXPECT_TRUE(toleranceEquals(x.getDeclination(), z.getDeclination(), TOL));
	Orientation th1 = x.makeTrue();
	EXPECT_FALSE(th1.isMagnetic());
	EXPECT_TRUE(toleranceEquals(th1.heading, x.heading + expectedDeclination, 0.25));
	Orientation th2 = th1.makeTrue();
	EXPECT_FALSE(th2.isMagnetic());
	EXPECT_TRUE(toleranceEquals(th1.heading, th2.heading, TOL));
	VLOG(2) << "Orientation th1 " << th1.pack();
	VLOG(2) << "Orientation th2 " << th2.pack();
}

TEST (Orientation, MakeMagnetic) {
	VLOG(1) << "===Orientation Declination (Make Magnetic) Test===";
	// location and orientation values 
	double expectedDeclination = 15.95;
	Orientation x { 20.0, -15.0, 30.0, false };
	Location y { 47.82167, -122.33639 };
	Orientation z { 0.0, 0.0, 240.0, false };
	VLOG(2) << "Orientation x " << x.pack();
	VLOG(2) << "Location y " << y.pack();
	VLOG(2) << "Orientation z " << z.pack();
	VLOG(2) << "Expected Declination " << expectedDeclination;
	
	EXPECT_FALSE(x.isMagnetic());
	EXPECT_TRUE(x.updateDeclination(y));
	VLOG(2) << "Declination is " << x.getDeclination() << ", " << z.getDeclination();
	EXPECT_TRUE(toleranceEquals(x.getDeclination(), z.getDeclination(), TOL));
	Orientation th1 = x.makeMag();
	EXPECT_FALSE(x.isMagnetic());
	EXPECT_TRUE(th1.isMagnetic());
	EXPECT_TRUE(toleranceEquals(th1.heading, x.heading - expectedDeclination, 0.25));
	Orientation th2 = th1.makeMag();
	EXPECT_TRUE(th2.isMagnetic());
	EXPECT_TRUE(toleranceEquals(th1.heading, th2.heading, TOL));
	VLOG(2) << "Orientation th1 " << th1.pack();
	VLOG(2) << "Orientation th2 " << th2.pack();
}