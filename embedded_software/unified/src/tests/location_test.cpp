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
	
	ASSERT_TRUE(good.isValid());
	ASSERT_FALSE(bad.isValid());
	bad.lat = 1;
	ASSERT_FALSE(bad.isValid());
	bad.lon = -179;
	ASSERT_TRUE(bad.isValid());
	bad.lon = -181;
	ASSERT_FALSE(bad.isValid());
	bad.lon = 179;
	ASSERT_TRUE(bad.isValid());
	bad.lon = 181;
	ASSERT_FALSE(bad.isValid());
	bad.lon = -140;
	ASSERT_TRUE(bad.isValid());
	bad.lat = 91;
	ASSERT_FALSE(bad.isValid());
	bad.lat = 89;
	ASSERT_TRUE(bad.isValid());
	bad.lat = -91;
	ASSERT_FALSE(bad.isValid());
	bad.lat = -89;
	ASSERT_TRUE(bad.isValid());
	bad.lat = 6;
	ASSERT_TRUE(bad.isValid());
}

TEST (Location, JSON) {
	Location u {3 , -4};
	Location v;
	json_t *loc;
	
	loc = u.pack();
	v.parse(loc);
	ASSERT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
	ASSERT_TRUE(toleranceEquals(u.lat, v.lat, TOL));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleBearing) {
	Location lax LAX;
	Location jfk JFK;
	ASSERT_EQ((int)round(lax.bearing(jfk)), 66);
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineBearing) {
	Location lax LAX;
	Location jfk JFK;
	ASSERT_EQ((int)round(lax.bearing(jfk, CourseTypeEnum::RhumbLine)), 79);
	ASSERT_EQ((int)round(jfk.bearing(lax, CourseTypeEnum::RhumbLine)), (79-180));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleDistance) {
	Location lax LAX;
	Location jfk JFK;
	ASSERT_TRUE(toleranceEquals((lax.distance(jfk)/NMTOM), 2144.0, 1));
	ASSERT_TRUE(toleranceEquals((jfk.distance(lax)/NMTOM), 2144.0, 1));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineDistance) {
	Location lax LAX;
	Location jfk JFK;
	ASSERT_TRUE(toleranceEquals((lax.distance(jfk, CourseTypeEnum::RhumbLine)/NMTOM), 2164.0, 1));
	ASSERT_TRUE(toleranceEquals((jfk.distance(lax, CourseTypeEnum::RhumbLine)/NMTOM), 2164.0, 1));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleTarget) {
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	vec = lax.target(jfk);
	ASSERT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2144.0, 1));
	ASSERT_EQ((int)round(vec.angleDeg()), 66);
	vec = jfk.target(lax);
	ASSERT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2144.0, 1));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineTarget) {
	Location lax LAX;
	Location jfk JFK;
	TwoVector vec;
	vec = lax.target(jfk, CourseTypeEnum::RhumbLine);
	ASSERT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2164.0, 1));
	ASSERT_EQ((int)round(vec.angleDeg()), 79);
	vec = jfk.target(lax, CourseTypeEnum::RhumbLine);
	ASSERT_TRUE(toleranceEquals((vec.mag()/NMTOM), 2164.0, 1));
	ASSERT_EQ((int)round(vec.angleDeg()), (79-180));
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, GreatCircleProject) {
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(66, (100 * NMTOM));
	loc = lax.project(vec);
	ASSERT_TRUE(toleranceEquals(loc.lat, (34.0 + (37.0/60.0)), 0.01));
	ASSERT_TRUE(toleranceEquals(loc.lon, (116.0 + (33.0/60.0)), 0.01));
	
}

// Taken from the worked examples in the Aviation Formulary
TEST (Location, RhumbLineProject) {
	Location lax LAX;
	Location jfk JFK;
	Location loc;
	TwoVector vec = TwoVector::getVectorDeg(79.3, (2164 * NMTOM));
	loc = lax.project(vec);
	ASSERT_TRUE(toleranceEquals(loc.lat, jfk.lat, 0.1));
	ASSERT_TRUE(toleranceEquals(loc.lon, jfk.lat, 0.1));
	
}