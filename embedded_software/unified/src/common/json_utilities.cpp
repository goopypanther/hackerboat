/******************************************************************************
 * JSON utilities
 *
 * See json_utilities.hpp for usage
 *
 ******************************************************************************/

#include "json_utilities.hpp"
#include <limits>

json_t *json(std::string const v)
{
	return json_stringn(v.data(), v.length());
}

json_t *json(bool v) {
	return json_boolean(v);
}

bool parse(json_t *j, std::string *s) {
	if (!json_is_string(j))
		return false;
	const char *p = json_string_value(j);
	size_t l = json_string_length(j);
	s->assign(p, l);
	return true;
}

bool parse(json_t *j, bool *b) {
	if (!j)
		return false;
	switch(json_typeof(j)) {
	case JSON_TRUE:
		*b = true;
		return true;
	case JSON_FALSE:
		*b = false;
		return true;
	default:
		return false;
	}
}

template<typename T>
static inline bool parsei(json_t *j, T *i) {
	if (j && json_is_integer(j)) {
		json_int_t v = json_integer_value(j);
		if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max())
			return false;
		*i = v;
		return true;
	}

	return false;
}

bool parse(json_t *j, signed char *i)	{ return parsei(j, i); }
bool parse(json_t *j, short *i)		{ return parsei(j, i); }
bool parse(json_t *j, int *i)   	{ return parsei(j, i); }
bool parse(json_t *j, long *i)  	{ return parsei(j, i); }
bool parse(json_t *j, unsigned char *i)  { return parsei(j, i); }
bool parse(json_t *j, unsigned short *i) { return parsei(j, i); }
bool parse(json_t *j, long long *i)  	{ return parsei(j, i); }
