#include <stdexcept>
#include <gtest/gtest.h>
#include <chrono>
#include "pid.hpp"
#include "test_utilities.hpp"

#define TOL 0.000001

TEST(PID, Construction) {
	double input, output, setpoint;
	PID mypid(&input, &output, &setpoint, 1.0, 0.1, 0.01, DIRECT);
	EXPECT_EQ(mypid.GetKp(), 1.0);
	EXPECT_EQ(mypid.GetKi(), 0.1);
	EXPECT_EQ(mypid.GetKd(), 0.01);
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], 1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 0.01, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], 0.1, TOL));
	EXPECT_EQ(mypid.GetDirection(), DIRECT);
	EXPECT_EQ(mypid.GetMode(), MANUAL);
}

TEST(PID, Accessors) {
	double input, output, setpoint;
	PID mypid(&input, &output, &setpoint, 1.0, 0.1, 0.01, DIRECT);
	mypid.SetTunings(100.0, 10.0, 1.0);
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], 100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], 10.0, TOL));
	EXPECT_EQ(mypid.GetDirection(), DIRECT);
	EXPECT_EQ(mypid.GetMode(), MANUAL);
	mypid.SetControllerDirection(REVERSE);
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], -100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], -1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], -10.0, TOL));
	mypid.SetSampleTime(10);
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], -100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], -0.1, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], -100.0, TOL));
}

TEST(PID, Limits) {
	double input = 0, output = 0, setpoint = 0;
	PID mypid(&input, &output, &setpoint, 1.0, 0, 0, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = 5.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = 6.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = -5.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
	input = -6.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
}

TEST(PID, Proportional) {
	double input = 0, output = 0, setpoint = 0;
	PID mypid(&input, &output, &setpoint, 1.0, 0, 0, DIRECT);
	mypid.SetOutputLimits(-100.0, 100.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	mypid.SetControllerDirection(REVERSE);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
}

TEST(PID, Integral) {
	double input = 0, output = 0, setpoint = 0;
	PID mypid(&input, &output, &setpoint, 0.0, 10.0, 0, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	for (int i = 1; i <= 5; i++) {
		EXPECT_TRUE(mypid.Compute());
		EXPECT_TRUE(toleranceEquals(output, (0 - i), TOL));
	}
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = -1;
	for (int i = -5; i <= 5; i++) {
		EXPECT_TRUE(toleranceEquals(output, i, TOL));
		EXPECT_TRUE(mypid.Compute());
	}
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
}

TEST(PID, Differential) {
	double input = 0, output = 0, setpoint = 0;
	PID mypid(&input, &output, &setpoint, 0.0, 0.0, 0.1, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
}

TEST(PID, Timing) {
	double input = 0, output = 0, setpoint = 0;
	PID mypid(&input, &output, &setpoint, 0.0, 10.0, 0, DIRECT);
	mypid.SetOutputLimits(-50.0, 50.0);
	mypid.SetSampleTime(10);
	mypid.SetMode(AUTOMATIC);
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 0.1, TOL));
	time_point<PID_CLOCK> testStart = PID_CLOCK::now();
	input = 1.0;
	while (PID_CLOCK::now() < (testStart + 99ms)) {
		mypid.Compute();
	}
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	
}