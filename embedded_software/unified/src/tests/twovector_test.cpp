#include <stdexcept>
#include <gtest/gtest.h>
#include <jansson.h>
#include "twovector.hpp"
#include "test_utilities.hpp"
#include "easylogging++.h"

#define TOL 0.0000001	// Tolerance for floating point comparisons

TEST (TwoVector, Creation) {
	VLOG(1) << "===TwoVector Creation Test===";
	TwoVector u;
	TwoVector v { 1 , 0 };
	TwoVector s { 3 , 4 };
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
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
	VLOG(1) << "===TwoVector Angle/Magnitude Output Test===";
	TwoVector v { 1 , 0 };
	TwoVector s { 3 , 4 };
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << s.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(s.angleDeg(), 53.1301023, TOL));
}

TEST (TwoVector, PolarInterpretation) {
	VLOG(1) << "===TwoVector Polar Interpretation Test===";
	TwoVector u;
	TwoVector v { 1 , 0 };
	
	v.x(3.0);
	VLOG(2) << "Setting v.x = 3";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.y(4);
	VLOG(2) << "Setting v.y = 4";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 53.1301023, TOL));
	
	u.x(1.0);
	VLOG(2) << "Setting u.x = 1";
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), 1.0, TOL));
	ASSERT_TRUE(isnan(u.y()));
	ASSERT_TRUE(isnan(u.mag()));
	ASSERT_TRUE(isnan(u.angleRad()));
	ASSERT_TRUE(isnan(u.angleDeg()));
	
	u.y(1.0);
	VLOG(2) << "Setting u.y = 1";
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.mag(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(u.angleRad(), M_PI_4, TOL));
	ASSERT_TRUE(toleranceEquals(u.angleDeg(), 45.0, TOL));
}

TEST (TwoVector, AngleMagInput) {
	VLOG(1) << "===TwoVector Angle/Magnitude Input Test===";
	TwoVector v { 1 , 0 };
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.mag(5.0);
	VLOG(2) << "Setting magnitude of v to 5.0";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	
	v.angleRad(atan2(4, 3));
	VLOG(2) << "Setting angle of v to atan2(4,3)";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 3.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), 0.9272952, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 53.1301023, TOL));
	
	v.angleRad(atan2(3, 4));
	VLOG(2) << "Setting angle of v to atan2(3,4)";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 5.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 3.0, TOL));
}

TEST (TwoVector, GetVectorRad) {
	VLOG(1) << "===TwoVector getVectorRad Test===";
	TwoVector v = TwoVector::getVectorRad(M_PI_4, 2.0);
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";	
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleRad(), M_PI_4, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
}

TEST (TwoVector, VectorRotations) {
	VLOG(1) << "===TwoVector Vector Rotations Test===";
	TwoVector v {2, 0};
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 90.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 2.0, TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 135.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 180.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -135.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -90.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -2.0, TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), -45.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), sqrt(2.0), TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -sqrt(2.0), TOL));
	v.rotateDeg(45.0);
	VLOG(2) << "Rotating v through +45 degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.angleDeg(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));
}

TEST (TwoVector, VectorAddition) {
	VLOG(1) << "===TwoVector Vector Addition Test===";
	TwoVector u {2, 0};
	TwoVector v {0, 1};
	TwoVector s;
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	
	u += v;
	VLOG(2) << "u += v";
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	
	v -= u;
	VLOG(2) << "v -= u";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	
	s = u + v;
	VLOG(2) << "s = u + v";
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << s.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	ASSERT_TRUE(toleranceEquals(s.x(), 0.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 1.0, TOL));	
	
	s = u - v;
	VLOG(2) << "s = u - v";
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << s.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), 2.0, TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), 1.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -2.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 0.0, TOL));	
	ASSERT_TRUE(toleranceEquals(s.x(), 4.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 1.0, TOL));	
}

TEST (TwoVector, ScalarMultiplication) {
	VLOG(1) << "===TwoVector Scalar Multiplication Test===";
	TwoVector s;
	TwoVector v {3, 4};
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	
	v *= 3;
	VLOG(2) << "v *= 3";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 9.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 12.0, TOL));	
	
	v *= -1;
	VLOG(2) << "v *= -1";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), -9.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), -12.0, TOL));

	v *= -(1.0/2.0); 
	VLOG(2) << "v *= -(1/2)";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	
	s = v * 2;
	VLOG(2) << "s = v * 2";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << s.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 15.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.x(), 9.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 12.0, TOL));
	
	s = v / 2;
	VLOG(2) << "s = v / 2";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << s.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(v.mag(), 7.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.x(), 4.5, TOL));
	ASSERT_TRUE(toleranceEquals(v.y(), 6.0, TOL));
	ASSERT_TRUE(toleranceEquals(s.mag(), 3.75, TOL));
	ASSERT_TRUE(toleranceEquals(s.x(), 2.25, TOL));
	ASSERT_TRUE(toleranceEquals(s.y(), 3, TOL));
}

TEST (TwoVector, DotProduct) {
	VLOG(1) << "===TwoVector Dot Product Test===";
	TwoVector s {1, 0};
	TwoVector u {0, 1};
	TwoVector v {3, 4};
	double w;
	
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	VLOG(2) << "Vector s is " << std::string(json_dumps(s.pack(), 0));
	VLOG(2) << "Vector s magnitude is " << v.mag();
	VLOG(2) << "Vector s angle is " << s.angleRad() << " radians or " << s.angleDeg() << " degrees";
	
	w  = v * s;
	VLOG(2) << "v dot s equals " << w;
	ASSERT_TRUE(toleranceEquals(w, 3, TOL));
	
	w  = v * u;
	VLOG(2) << "v dot u equals " << w;
	ASSERT_TRUE(toleranceEquals(w, 4, TOL));
}

TEST (TwoVector, JSON) {
	VLOG(1) << "===TwoVector JSON Test===";
	TwoVector u {3 , -4};
	TwoVector v;
	json_t *vec;
	
	vec = u.pack();
	VLOG(2) << "Vector u is " << std::string(json_dumps(u.pack(), 0));
	VLOG(2) << "Vector u magnitude is " << u.mag();
	VLOG(2) << "Vector u angle is " << u.angleRad() << " radians or " << u.angleDeg() << " degrees";
	VLOG(2) << "Vector u JSON: " << std::string(json_dumps(vec, 0));
	v.parse(vec);
	VLOG(2) << "Populating v from s.pack()";
	VLOG(2) << "Vector v is " << std::string(json_dumps(v.pack(), 0));
	VLOG(2) << "Vector v magnitude is " << v.mag();
	VLOG(2) << "Vector v angle is " << v.angleRad() << " radians or " << v.angleDeg() << " degrees";
	ASSERT_TRUE(toleranceEquals(u.x(), v.x(), TOL));
	ASSERT_TRUE(toleranceEquals(u.y(), v.y(), TOL));
}