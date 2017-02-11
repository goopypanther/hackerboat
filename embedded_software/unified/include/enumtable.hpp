/**************************************************************************//**
 * @brief  Enumeration name table
 * @file   enumtable.hpp
 * 
 * A utility class for holding a statically-defined bidirectional
 * integer/string mapping.
 *
 * Written by Wim Lewis, Mar 2016
 *
 ******************************************************************************/

#ifndef ENUMTABLE_H
#define ENUMTABLE_H

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

/** Integer-to-name mapping
 *
 * Provides a bidirectional mapping between integers 0..N and string names.
 */
class IntNameTable {
public:
	/** Constructs an integer name table given a list of names */
	IntNameTable(std::initializer_list<const char *> names);

	/** Returns true if the integer is within the range of numbers in the table */
	bool valid(int) const;

	/** Retrieve the string name for a given number.
	 * @throws std::out_of_range
	 */
	const std::string& get(int) const;

protected:
	const std::vector<std::string> forward;
	const std::unordered_map<std::string, int> backward;

	const std::unordered_map<std::string, int>::const_iterator find(const std::string& str) const;

	IntNameTable(const std::pair<std::vector<std::string>, std::unordered_map<std::string, int> >&& values)
		: forward(values.first), backward(values.second)
	{
	};
};

/**
 * EnumNameTable is a purely inline specialization of
 * IntNameTable which provides methods for strongly-typed enums
 * ("enum class").
 *
 * \tparam T The enumeration type.
 */

template<typename T>
class EnumNameTable : public IntNameTable {
public:
	// Explicitly inherit our constructor
	// using IntNameTable::IntNameTable; // Doesn't work until g++-4.8, unfortunately
	EnumNameTable(std::initializer_list<const char *> names)
	  : IntNameTable(names)
	{};
	

	// Explanation for other people who are (like me) not super
	// familiar with C++: Type-based function overloading only
	// works among functions in the same scope, and a subclass's
	// methods are in a different scope than its superclass'. So,
	// to keep the enum-specialized versions of get/valid from
	// hiding the unspecialized versions, we explicitly bring our
	// parent's versions of those methods into scope here.
	using IntNameTable::valid;

	/** Retrieve the enumeration for a string name.
	 *
	 * @param[in]  str  A string naming an enumerated value
	 * @param[out] num  The value corresponding to str
	 * @returns         true if the name corresponds to a value; false otherwise
	 */
	bool get(const std::string& str, T *num) const {
		auto p = find(str);
		if (p == backward.cend()) {
			return false;
		} else {
			*num = static_cast<T>(p->second);
			return true;
		}
	};

	/** Retrieve the string name of an enumerated value
	 * @throws std::out_of_range
	 */
	const std::string& get(T num) const {
		return IntNameTable::get(static_cast<int>(num));
	}

	bool valid(T num) const {
		return valid(static_cast<int>(num));
	}
};

#endif /* ENUMTABLE_H */
