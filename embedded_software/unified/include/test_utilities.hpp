
#include <gtest/gtest.h>
#include <iostream>

testing::AssertionResult assertJSONEqual(const char *expected_expr, const char *actual_expr,
					 json_t *expected, json_t *actual);

#define EXPECT_JSON_EQ(a, b) EXPECT_PRED_FORMAT2(assertJSONEqual, a, b)
#define ASSERT_JSON_EQ(a, b) ASSERT_PRED_FORMAT2(assertJSONEqual, a, b)

bool inline toleranceEquals (double a, double b, double tolerance) {
	double diff = a - b;
	if (abs(diff) < tolerance) {
		return true;
	} else {
		std::cerr << a << " " << b << " " << diff << std::endl;
		return false;
	}
}