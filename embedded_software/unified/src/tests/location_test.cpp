#include <stdexcept>
#include <gtest/gtest.h>
#include <jansson.h>
#include <cmath>
#include "location.hpp"
#include "twovector.hpp"
#include "test_utilities.hpp"

#define TOL (0.00001)	// Tolerance for floating point comparisons
#define NMTOM (1852)
#define JFK {(40 + (38/60)), -(73 + (47/60))}
#define LAX {(33 + (57/60)), -(118 + (24/60))}

TEST (Location, Creation) {
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
	Location u {3 , -4};
	Location v;
	json_t *loc;
	
	loc = u.pack();
	v.parse(loc);
	EXPECT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
	EXPECT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleBearing) {
	Location lax LAX;
	Location jfk JFK;
	EXPECT_EQ((int)round(lax.bearing(jfk)), 66);
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineBearing) {
	Location lax LAX;
	Location jfk JFK;
	EXPECT_EQ((int)round(lax.bearing(jfk, CourseTypeEnum::RhumbLine)), 79);
	EXPECT_EQ((int)round(jfk.bearing(lax, CourseTypeEnum::RhumbLine)), (79-180));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleDistance) {
	Location lax LAX;
	Location jfk JFK;
	EXPECT_TRUE(toleranceEquals((lax.distance(jfk)/NMTOM), 2193.0, 1));
	EXPECT_TRUE(toleranceEquals((jfk.distance(lax)/NMTOM), 2193.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineDistance) {
	Location lax LAX;
	Location jfk JFK;
	EXPECT_TRUE(toleranceEquals((lax.distance(jfk, CourseTypeEnum::RhumbLine)/NMTOM), 2214.0, 1));
	EXPECT_TRUE(toleranceEquals((jfk.distance(lax, CourseTypeEnum::RhumbLine)/NMTOM), 2214.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleTarget) {
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	vec = lax.target(jfk);
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2193.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), 66);
	vec = jfk.target(lax);
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2193.0, 1));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineTarget) {
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	vec = lax.target(jfk, CourseTypeEnum::RhumbLine);
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2214.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), 79);
	vec = jfk.target(lax, CourseTypeEnum::RhumbLine);
	EXPECT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2214.0, 1));
	EXPECT_EQ((int)round(vec.angleDeg()), (79-180));
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, GreatCircleProject) {
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(66, (100 * NMTOM));
	loc = lax.project(vec);
	EXPECT_TRUE(toleranceEquals(loc.lat, 33.666, 0.01));
	EXPECT_TRUE(toleranceEquals(loc.lon, -116.176, 0.01));
	
}

// Taken from the worked examples in the Aviation Formulary, modified to get the right answers with GeographicLib
TEST (Location, RhumbLineProject) {
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(79, (2214 * NMTOM));
	loc = lax.project(vec);
	EXPECT_TRUE(toleranceEquals(loc.lat, 32.1341, 0.1));
	EXPECT_TRUE(toleranceEquals(loc.lon, -74.0216, 0.1));
	
}