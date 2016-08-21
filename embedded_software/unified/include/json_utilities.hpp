/**************************************************************************//**
 * @file json_utilities.hpp
 *
 * @brief Hackerboat JSON utilities
 *
 *
 * Overridden helper functions for translating data to and from JSON representations.
 *
 ******************************************************************************/

#ifndef HACKERBOAT_JSON_UTILITIES_H
#define HACKERBOAT_JSON_UTILITIES_H

extern "C" {
#include <jansson.h>
}
#include <type_traits>
#include <string>
#include <ostream>

/** @fn json_t *json(T foo)
 *
 * @brief Create a JSON representation
 *
 * @param[in] foo  The value to converto to JSON
 * @return         A new reference
 *
 * Returns a new reference to a json representation of foo. It must be
 * decref'd by its caller (presumably implicitly as part of creating a
 * json object containing the new value). It is overloaded for various
 * types.
 */
json_t *json(std::string const);
json_t *json(bool v);
inline json_t *json(const char *s) {
	return json_string(s);
}
inline json_t *json(json_int_t i) {
	return json_integer(i);
}
inline json_t *json(double v) {
	return json_real(v);
}

/** @fn bool parse(json_t *obj, T *var)
 *
 * @brief Set a variable based on a JSON representation
 *
 * @param[in]  obj The JSON object to parse. May be NULL.
 * @param[out] var A pointer to the variable to set. Must not be NULL.
 * @return true if the conversion succeeded; false otehrwise.
 *
 */
bool parse(json_t *, std::string *);
bool parse(json_t *, bool *);

bool parse(json_t *, signed char *);
bool parse(json_t *, short *);
bool parse(json_t *, unsigned short *);
bool parse(json_t *, int *);
bool parse(json_t *, unsigned int *);
bool parse(json_t *, long *);
bool parse(json_t *, long long *);

/* Generics using C++'s ridiculous metaprogramming syntax. The first
   one dispatches to fromString() if possible, and the second one
   handles floating-point types. */
template<typename T,
	 typename = typename std::enable_if< !( std::is_arithmetic<T>::value ) >::type
	 >
static inline bool parse(json_t *j, T *v) {
	/* json_string_value() detects NULL and non-strings and returns NULL */
	const char *strvalue = json_string_value(j);
	if (!strvalue)
		return false;
	return fromString(strvalue, v);
}
template<typename T,
	 typename = typename std::enable_if<std::is_floating_point<T>::value>::type,
	 typename Dummy = float
	 >
static inline bool parse(json_t *j, T *v) {
	if (!json_is_number(j)) {
		return false;
	} else {
		*v = json_number_value(j);
		return true;
	}
}

extern "C" {
	/**
	 * @brief Add multiple values to an object
	 *
	 * This is similar to json_pack(), but allows adding
	 * additional items to an existing object, and uses a simpler
	 * format string (it contains only the types of the values,
	 * not the keys, which are always <tt>char *</tt>.)
	 *
	 * The format string may contain spaces for
	 * grouping/readability, which are ignored.
	 *
	 * @param[inout] obj   The JSON object to modify
	 * @param[in] types    The types of the values being added.
	 * @return             0 on succes, nonzero on failure.
	 */
	int json_object_add(json_t *obj, const char *types, ...);

	/* Internal function used by the operator<< inline */
	int jansson_ostream(const char *buffer, size_t size, void *ctxt);
}

/** @brief Write the object in JSON format to the ostream.
 */
static inline
std::ostream& operator<< (std::ostream& os, json_t *j)
{
	json_dump_callback(j, jansson_ostream, static_cast<void *>(&os), JSON_ENCODE_ANY);
	return os;
}

#endif /* HACKERBOAT_JSON_UTILITIES_H */
