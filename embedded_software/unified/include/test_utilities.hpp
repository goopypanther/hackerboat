
#include <gtest/gtest.h>

testing::AssertionResult assertJSONEqual(const char *expected_expr, const char *actual_expr,
					 json_t *expected, json_t *actual);

#define EXPECT_JSON_EQ(a, b) EXPECT_PRED_FORMAT2(assertJSONEqual, a, b)
#define ASSERT_JSON_EQ(a, b) ASSERT_PRED_FORMAT2(assertJSONEqual, a, b)

