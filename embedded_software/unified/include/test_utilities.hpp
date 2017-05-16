
#include <gtest/gtest.h>
#include <iostream>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cmath>

#ifndef TEST_UTILITIES
#define TEST_UTILITIES

/*testing::AssertionResult assertJSONEqual(const char *expected_expr, 
										const char *actual_expr,
					 					Value& expected, Value& actual);

#define EXPECT_JSON_EQ(a, b) EXPECT_PRED_FORMAT2(assertJSONEqual, a, b)
#define ASSERT_JSON_EQ(a, b) ASSERT_PRED_FORMAT2(assertJSONEqual, a, b)*/


bool inline toleranceEquals (double a, double b, double tolerance) {	/**< Compare two doubles with a tolerance */
	double diff = a - b;
	if (fabs(diff) < tolerance) {
		return true;
	} else {
		std::cerr << a << " " << b << " " << diff << std::endl;
		return false;
	}
}

#endif /* TEST_UTILITIES */