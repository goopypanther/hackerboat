#include <stdexcept>
#include <gtest/gtest.h>
#include <jansson.h>
#include "twovector.hpp"
#include "test_utilities.hpp"

#define TOL 0.0000001	// Tolerance for floating point comparisons

TEST (TwoVector, Creation) {
	TwoVector u;
	TwoVector v { 1 , 0 };
	TwoVector s { 3 , 4 };
	ASSERT_FALSE(u.isValid());
	ASSERT_TRUE(v.isValid());
	ASSERT_TRUE(s.isValid());
	ASSERT_TRUE(isnan(u.x()));
	ASSERT_TRUE(isnan(u.y()));
	ASSERT_TRUE(toleranceEquals(v.x(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 4.0, TOL));
}

TEST (TwoVector, AngleMagOutput) {
	TwoVector v { 1 , 0 };
	TwoVector s { 3 , 4 };
	ASSERT_TRUE(toleranceEquals(v.mag(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(s.angleDeg(), 53.1301023, TOL));
}

TEST (TwoVector, PolarInterpretation) {
	TwoVector u;
	TwoVector v { 1 , 0 };
	
	v.x(3.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.y(4);
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 53.1301023, TOL));
	
	u.x(1.0);
	ASSERT_TRUE(toleranceEquals(u.x(), 1.0, TOL));
	ASSERT_TRUE(isnan(u.y()));
	ASSERT_TRUE(isnan(u.mag()));
	ASSERT_TRUE(isnan(u.angleRad()));
	ASSERT_TRUE(isnan(u.angleDeg()));
	
	u.y(1.0);
	ASSERT_TRUE(toleranceEquals(u.x(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.mag(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(u.angleRad(), M_PI_4, TOL));
	ASSERT_TRUE(toleranceEquals(u.angleDeg(), 45.0, TOL));
}

TEST (TwoVector, AngleMagInput) {
	TwoVector v { 1 , 0 };
	ASSERT_TRUE(toleranceEquals(v.mag(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.mag(5.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.angleRad(atan2(4, 3));
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 53.1301023, TOL));
	
	v.angleRad(atan2(3, 4));
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 3.0, TOL));
}

TEST (TwoVector, GetVectorRad) {
	TwoVector v = TwoVector::getVectorRad(M_PI_4, 2.0);
	
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), M_PI_4, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
}

TEST (TwoVector, VectorRotations) {
	TwoVector v {2, 0};
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 90.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 2.0, TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 135.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 180.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -135.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -90.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -2.0, TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
}

TEST (TwoVector, VectorAddition) {
	TwoVector u {2, 0};
	TwoVector v {0, 1};
	TwoVector s;
	
	u += v;
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	
	v -= u;
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	
	s = u + v;
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	ASSERT_TRUE(toleranceEquals(s.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 1.0, TOL));	
	
	s = u - v;
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	ASSERT_TRUE(toleranceEquals(s.x(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 1.0, TOL));	
}

TEST (TwoVector, ScalarMultiplication) {
	TwoVector s;
	TwoVector v {3, 4};
	
	v *= 3;
	ASSERT_TRUE(toleranceEquals(v.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 9.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 12.0, TOL));	
	
	v *= -1;
	ASSERT_TRUE(toleranceEquals(v.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -9.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -12.0, TOL));

	v *= -(1.0/2.0); 
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	
	s = v * 2;
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.x(), 9.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 12.0, TOL));
	
	s = v / 2;
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 3.75, TOL));
	ASSERT_TRUE(toleranceEquals(s.x(), 2.25, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 3, TOL));
}

TEST (TwoVector, DotProduct) {
	TwoVector s {1, 0};
	TwoVector u {0, 1};
	TwoVector v {3, 4};
	double w;
	
	w  = v * s;
	ASSERT_TRUE(toleranceEquals(w, 3, TOL));
	
	w  = v * u;
	ASSERT_TRUE(toleranceEquals(w, 4, TOL));
}

TEST (TwoVector, JSON) {
	TwoVector u {3 , -4};
	TwoVector v;
	json_t *vec;
	
	vec = u.pack();
	v.parse(vec);
	ASSERT_TRUE(toleranceEquals(u.x(), v.x(), TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), v.y(), TOL));
}