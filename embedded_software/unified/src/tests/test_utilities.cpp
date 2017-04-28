
extern "C" {
	#include <math.h>
}
#include <gtest/gtest.h>
#include <list>
#include "rapidjson/rapidjson.h"

using namespace rapidjson;

std::ostream& operator<< (std::ostream& os, pathelt *p)
{
	if (!p) {
		return os << "\t<root>";
	}

	if (p->prev) {
		os << p->prev << ".";
	} else {
		os << "\t";
	}
	return os << p->elt;
}

testing::AssertionResult assertJSONEqual(const char *expected_expr, const char *actual_expr,
					 Value& expected, Value& actual)
{
	//std::ostringstream diffs;
	//int n = json_describe_some_differences(0, diffs, expected, actual);

	if (expected == actual)
		return ::testing::AssertionSuccess();

	testing::AssertionResult result(testing::AssertionFailure());

	result << "  " << expected_expr << ": " << expected << std::endl;
	result << "  " << actual_expr   << ": " << actual   << std::endl;
	//result << diffs.str();

	return result;
}

