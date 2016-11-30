#include <stdexcept>
#include <gtest/gtest.h>
#include <chrono>
#include <tuple>
#include "pid.hpp"
#include "test_utilities.hpp"
#include "easylogging++.h"

#define TOL 0.000001

TEST(PID, Construction) {
	VLOG(1) << "===PID Construction Test===";
	double input, output, setpoint;
	PID mypid(&input, &output, &setpoint, 1.0, 0.1, 0.01, DIRECT);
	VLOG(2) << "Setting PID values to " << mypid.GetKp() << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_EQ(mypid.GetKp(), 1.0);
	EXPECT_EQ(mypid.GetKi(), 0.1);
	EXPECT_EQ(mypid.GetKd(), 0.01);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], 1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 0.01, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], 0.1, TOL));
	EXPECT_EQ(mypid.GetDirection(), DIRECT);
	EXPECT_EQ(mypid.GetMode(), MANUAL);
}

TEST(PID, Accessors) {
	VLOG(1) << "===PID Accessors Test===";
	double input, output, setpoint;
	PID mypid(&input, &output, &setpoint, 1.0, 0.1, 0.01, DIRECT);
	VLOG(2) << "Setting tunings to {100, 10, 1}";
	mypid.SetTunings(100.0, 10.0, 1.0);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], 100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], 10.0, TOL));
	EXPECT_EQ(mypid.GetDirection(), DIRECT);
	EXPECT_EQ(mypid.GetMode(), MANUAL);
	VLOG(2) << "Reversing direction...";
	mypid.SetControllerDirection(REVERSE);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], -100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], -1.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], -10.0, TOL));
	VLOG(2) << "Setting sample time to 10 ms";
	mypid.SetSampleTime(10);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], -100.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], -0.1, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], -100.0, TOL));
	std::tuple<double, double, double> K = {10.0, 1.0, 0.1};
	VLOG(2) << "Setting tunings to {10, 1, 0.1} with a tuple";
	mypid.SetTunings(K);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[0], -10.0, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], -0.01, TOL));
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[2], -10.0, TOL));
	
}

TEST(PID, Limits) {
	VLOG(1) << "===PID Limits Test===";
	double input = 0, output = 0, setpoint = 0;
	VLOG(2) << "Setting proportional to 1, direct acting, with limits of +/-5";
	PID mypid(&input, &output, &setpoint, 1.0, 0, 0, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = 5.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = 6.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = -5.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
	input = -6.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
}

TEST(PID, Proportional) {
	VLOG(1) << "===PID Proportional Test===";
	double input = 0, output = 0, setpoint = 0;
	VLOG(2) << "Setting proportional to 1, direct acting, with limits of +/-100";
	PID mypid(&input, &output, &setpoint, 1.0, 0, 0, DIRECT);
	mypid.SetOutputLimits(-100.0, 100.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	VLOG(2) << "Reversing controller direction";
	mypid.SetControllerDirection(REVERSE);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
}

TEST(PID, Integral) {
	VLOG(1) << "===PID Integral Test===";
	double input = 0, output = 0, setpoint = 0;
	VLOG(2) << "Setting integral to 10.0, 100 ms sample time, direct acting, with limits of +/-5";
	PID mypid(&input, &output, &setpoint, 0.0, 10.0, 0, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	for (int i = 1; i <= 5; i++) {
		EXPECT_TRUE(mypid.Compute());
		VLOG(2) << "Input: " << input << " output: " << output << " iteration: " << i;
		EXPECT_TRUE(toleranceEquals(output, (0 - i), TOL));
	}
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -5.0, TOL));
	input = -1;
	for (int i = -5; i <= 5; i++) {
		EXPECT_TRUE(toleranceEquals(output, i, TOL));
		EXPECT_TRUE(mypid.Compute());
		VLOG(2) << "Input: " << input << " output: " << output << " iteration: " << i;
	}
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 5.0, TOL));
}

TEST(PID, Differential) {
	VLOG(1) << "===PID Differential Test===";
	double input = 0, output = 0, setpoint = 0;
	VLOG(2) << "Setting differential to 0.1, 100 ms sample time, direct acting, with limits of +/-5";
	PID mypid(&input, &output, &setpoint, 0.0, 0.0, 0.1, DIRECT);
	mypid.SetOutputLimits(-5.0, 5.0);
	input = 1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	input = 2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	input = -1.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 1.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	input = -2.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 2.0, TOL));
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, 0.0, TOL));
	input = 0.0;
	EXPECT_TRUE(mypid.Compute());
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -2.0, TOL));
}

TEST(PID, Timing) {
	VLOG(1) << "===PID Timing Test===";
	double input = 0, output = 0, setpoint = 0;
	VLOG(2) << "Setting integral to 10.0, 10 ms sample time, direct acting, with limits of +/-50";
	PID mypid(&input, &output, &setpoint, 0.0, 10.0, 0, DIRECT);
	mypid.SetOutputLimits(-50.0, 50.0);
	mypid.SetSampleTime(10);
	mypid.SetMode(AUTOMATIC);
	VLOG(2) << "Internal PID values are " << mypid.getRealK()[0] << ", " << mypid.GetKi() << ", " << mypid.GetKd(); 
	EXPECT_TRUE(toleranceEquals(mypid.getRealK()[1], 0.1, TOL));
	time_point<PID_CLOCK> testStart = PID_CLOCK::now();
	input = 1.0;
	while (PID_CLOCK::now() < (testStart + 99ms)) {
		mypid.Compute();
	}
	VLOG(2) << "Input: " << input << " output: " << output;
	EXPECT_TRUE(toleranceEquals(output, -1.0, TOL));
	
}