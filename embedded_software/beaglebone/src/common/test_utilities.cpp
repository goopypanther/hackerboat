
extern "C" {
#include <jansson.h>
#include <math.h>
}
#include <gtest/gtest.h>
#include <list>

class pathelt {
public:
	pathelt *prev;
	std::string elt;
	pathelt(pathelt *prev, std::string p) : prev(prev), elt(p) {};
};


static int json_describe_array_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right);
static int json_describe_object_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right);
static int json_describe_some_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right);
static int json_diffnums(pathelt *path, std::ostream& os, json_t *left, json_t *right, bool reversed);

std::ostream& operator<< (std::ostream& os, pathelt *p)
{
	if (p->prev) {
		os << p->prev << ".";
	} else {
		os << "\t";
	}
	return os << p->elt;
}

testing::AssertionResult assertJSONEqual(const char *expected_expr, const char *actual_expr,
					 json_t *expected, json_t *actual)
{
	std::ostringstream diffs;
	int n = json_describe_some_differences(0, diffs, expected, actual);

	if (n == 0)
		return ::testing::AssertionSuccess();

	testing::AssertionResult result(testing::AssertionFailure());

	result << "  " << expected_expr << ": " << expected << std::endl;
	result << "  " << actual_expr   << ": " << actual   << std::endl;
	result << diffs.str();

	return result;
}

static const char * const jansson_typenames[] = {
	[JSON_OBJECT] = "object",
	[JSON_ARRAY] = "array",
	[JSON_STRING] = "string",
	[JSON_INTEGER] = "integer",
	[JSON_REAL] = "real",
	[JSON_TRUE] = "true",
	[JSON_FALSE] = "false",
	[JSON_NULL] = "null"
};

static int json_describe_some_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right)
{
	if (left == right)
		return 0;

#if 1
	if (json_typeof(left) == JSON_INTEGER && json_typeof(right) == JSON_REAL)
		return json_diffnums(path, os, left, right, false);
	if (json_typeof(right) == JSON_INTEGER && json_typeof(left) == JSON_REAL)
		return json_diffnums(path, os, right, left, true);
#endif

	if (json_typeof(left) != json_typeof(right)) {
		os << path << ": " << jansson_typenames[json_typeof(left)] << " vs. " << jansson_typenames[json_typeof(right)] << std::endl;
		return 1;
	}

	switch (json_typeof(left)) {
	case JSON_TRUE:
	case JSON_FALSE:
	case JSON_NULL:
		/* Singleton types */
		return 0;

	case JSON_INTEGER:
		{
			json_int_t lv = json_integer_value(left);
			json_int_t rv = json_integer_value(right);
			if (lv != rv) {
				os << path << ": integer " << lv << " vs. " << rv << std::endl;
				return 1;
			} else {
				return 0;
			}
		}

	case JSON_REAL:
		{
			double lv = json_real_value(left);
			double rv = json_real_value(right);
			if (lv != rv) {
				os << path << ": real " << lv << " vs. " << rv << " (difference=" << (rv-lv) << ")" << std::endl;
				return 1;
			} else {
				return 0;
			}
		}

	case JSON_STRING:
		if (!json_equal(left, right)) {
			os << path << ": strings \"" << json_string_value(left) << "\" vs. \"" << json_string_value(right) << "\"" << std::endl;
			return 1;
		} else {
			return 0;
		}

	case JSON_ARRAY:
		return json_describe_array_differences(path, os, left, right);
	case JSON_OBJECT:
		return json_describe_object_differences(path, os, left, right);

	default:
		abort();
	}
}

static int json_diffnums(pathelt *path, std::ostream& os, json_t *left, json_t *right, bool reversed)
{
	json_int_t lv = json_integer_value(left);
	double rv = json_real_value(right);

	if (rv != ::floor(rv) ||
	    lv != rv) {
		std::string l("integer ");
		l += std::to_string(lv);
		std::string r("real ");
		r += std::to_string(rv);
		if (reversed)
			std::swap(l, r);
		os << path << ": numbers " << l << " vs. " << r << std::endl;
		return 1;
	}

	return 0;
}


static int json_describe_array_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right)
{
	size_t lv = json_array_size(left);
	size_t rv = json_array_size(left);
	if (lv != rv) {
		os << path << ": array has " << lv << " elements vs. " << rv << " elements" << std::endl;
		return 1;
	} else {
		int count = 0;
		for(size_t i = 0; i < lv; i++) {
			pathelt subp(path, std::string("[") + std::to_string(i) + std::string("]"));
			count += json_describe_some_differences(&subp, os,
								json_array_get(left, i),
								json_array_get(right, i));
			if (count > 2) {
				os << path << " ..." << std::endl;
				break;
			}
		}
		return count;
	}
}

static int json_describe_object_differences(pathelt *path, std::ostream& os, json_t *left, json_t *right)
{
	std::list<std::string> missing_left;
	std::list<std::string> missing_right;
	const char *key;
	json_t *value;
	int count = 0;

	json_object_foreach(left, key, value) {
		if (!json_object_get(right, key)) {
			missing_right.emplace_back(key);
			count ++;
		}
	}
	json_object_foreach(right, key, value) {
		if (!json_object_get(left, key)) {
			missing_left.emplace_back(key);
			count ++;
		}
	}

	if (count) {
		os << path << ": " << (missing_left.size()) << "/" << (missing_right.size()) << " missing keys:";
		for (auto it = missing_left.cbegin(); it != missing_left.cend(); it ++) {
			os << " " << (*it);
		}
		os << " /";
		for (auto it = missing_right.cbegin(); it != missing_right.cend(); it ++) {
			os << " " << (*it);
		}
		os << std::endl;
		return 1;
	}

	json_object_foreach(left, key, value) {
		json_t *right_value = json_object_get(right, key);
		pathelt subp(path, std::string(key));
		count += json_describe_some_differences(&subp, os, value, right_value);
		if (count > 4) {
			if (count < 5) {
				os << path << " ..." << std::endl;
				count ++;
			}
			break;
		}
	}

	return count;
}

