#include <stdexcept>
#include <gtest/gtest.h>
#include <jansson.h>
#include <cmath>
#include "location.hpp"
#include "twovector.hpp"
#include "test_utilities.hpp"
#include "easylogging++.h"

#define TOL (0.00001)	// Tolerance for floating point comparisons
#define NMTOM (1852)
#define JFK {(40 + (38/60)), -(73 + (47/60))}
#define LAX {(33 + (57/60)), -(118 + (24/60))}

TEST (Location, Creation) {
	VLOG(1) << "===Location Test, Creation===";
	Location good {10.0, 10.0};
	Location bad;
	
	EXPECT_TRUE(good.isValid());
	EXPECT_FALSE(bad.isValid());
	bad.lat = 1;
	EXPECT_FALSE(bad.isValid());
	bad.lon = -179;
	EXPECT_TRUE(bad.isValid());
	bad.lon = -181;
	EXPECT_FALSE(bad.isValid());
	bad.lon = 179;
	EXPECT_TRUE(bad.isValid());
	bad.lon = 181;
	EXPECT_FALSE(bad.isValid());
	bad.lon = -140;
	EXPECT_TRUE(bad.isValid());
	bad.lat = 91;
	EXPECT_FALSE(bad.isValid());
	bad.lat = 89;
	EXPECT_TRUE(bad.isValid());
	bad.lat = -91;
	EXPECT_FALSE(bad.isValid());
	bad.lat = -89;
	EXPECT_TRUE(bad.isValid());
	bad.lat = 6;
	EXPECT_TRUE(bad.isValid());
}

TEST (Location, JSON) {
	VLOG(1) << "===Location Test, Creation===";
	Location u {3 , -4};
	Location v;
	json_t *loc;
	
	loc = u.pack();
	VLOG(2) << "Output JSON: " << json_dumps(loc, 0);
	v.parse(loc);
	VLOG(2) << "Parsed location: " << v;
	EXPECT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
	EXPECT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleBearing) {
	VLOG(1) << "===Location Test, Great Circle Bearing===";
	Location lax LAX;
	Location jfk JFK;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	VLOG(2) << "LAX bearing to JFK: " << lax.bearing(jfk);
	EXPECT_EQ((int)round(lax.bearing(jfk)), 66);
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineBearing) {
	VLOG(1) << "===Location Test, Rhumb Line Bearing===";
	Location lax LAX;
	Location jfk JFK;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	VLOG(2) << "LAX bearing to JFK: " << lax.bearing(jfk, CourseTypeEnum::RhumbLine);
	VLOG(2) << "JFK bearing to LAX: " << jfk.bearing(lax, CourseTypeEnum::RhumbLine);
	EXPECT_EQ((int)round(lax.bearing(jfk, CourseTypeEnum::RhumbLine)), 79);
	EXPECT_EQ((int)round(jfk.bearing(lax, CourseTypeEnum::RhumbLine)), (79-180));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleDistance) {
	VLOG(1) << "===Location Test, Great Circle Distance===";
	Location lax LAX;
	Location jfk JFK;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	VLOG(2) << "LAX distance to JFK: " << lax.distance(jfk) << " m";
	VLOG(2) << "JFK distance to LAX: " << jfk.distance(lax) << " m";
	EXPECT_TRUE(toleranceEquals((lax.distance(jfk)/NMTOM), 2193.0, 1));
	EXPECT_TRUE(toleranceEquals((jfk.distance(lax)/NMTOM), 2193.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineDistance) {
	VLOG(1) << "===Location Test, Rhumb Line Distance===";
	Location lax LAX;
	Location jfk JFK;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	VLOG(2) << "LAX distance to JFK: " << lax.distance(jfk, CourseTypeEnum::RhumbLine) << " m";
	VLOG(2) << "JFK distance to LAX: " << jfk.distance(lax, CourseTypeEnum::RhumbLine) << " m";
	EXPECT_TRUE(toleranceEquals((lax.distance(jfk, CourseTypeEnum::RhumbLine)/NMTOM), 2214.0, 1));
	EXPECT_TRUE(toleranceEquals((jfk.distance(lax, CourseTypeEnum::RhumbLine)/NMTOM), 2214.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleTarget) {
	VLOG(1) << "===Location Test, Great Circle Target===";
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	vec = lax.target(jfk);
	VLOG(2) << "LAX vector to JFK: " << vec;
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2193.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), 66);
	vec = jfk.target(lax);
	VLOG(2) << "JFK vector to LAX: " << vec;
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2193.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineTarget) {
	VLOG(1) << "===Location Test, Rhumb Line Target===";
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	VLOG(2) << "LAX: " << lax << ", JFK: " << jfk;
	vec = lax.target(jfk, CourseTypeEnum::RhumbLine);
	VLOG(2) << "LAX vector to JFK: " << vec;
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2214.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), 79);
	vec = jfk.target(lax, CourseTypeEnum::RhumbLine);
	VLOG(2) << "JFK vector to LAX: " << vec;
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2214.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), (79-180));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleProject) {
	VLOG(1) << "===Location Test, Great Circle Project===";
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(66, (100 * NMTOM));
	loc = lax.project(vec);
	VLOG(2) << "LAX: " << lax << ", 100 nm projection @ 66 deg" << loc;
	EXPECT_TRUE(toleranceEquals(loc.lat, 33.666, 0.01));
	EXPECT_TRUE(toleranceEquals(loc.lon, -116.176, 0.01));
	
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineProject) {
	VLOG(1) << "===Location Test, Rhumb Line Project===";
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(79, (2214 * NMTOM));
	loc = lax.project(vec);
	VLOG(2) << "LAX: " << lax << ", 2214 nm projection @ 79 deg" << loc;
	EXPECT_TRUE(toleranceEquals(loc.lat, 32.1341, 0.1));
	EXPECT_TRUE(toleranceEquals(loc.lon, -74.0216, 0.1));
	
}