/******************************************************************************
 * Enumeration name table
 * 
 * A utility class for holding a statically-defined bidirectional
 * integer/string mapping.
 *
 * Written by Wim Lewis, Mar 2016
 *
 ******************************************************************************/

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

/**
 *
 */

class integerNameTable {
public:
	integerNameTable(std::initializer_list<const char *> names);

	bool valid(int) const;

	const std::string& get(int) const;

protected:
	const std::vector<std::string> forward;
	const std::unordered_map<std::string, int> backward;

	const std::unordered_map<std::string, int>::const_iterator find(const std::string& str) const;

	integerNameTable(const std::pair<std::vector<std::string>, std::unordered_map<std::string, int> >&& values)
		: forward(values.first), backward(values.second)
	{
	};
};

/**
 * enumerationNameTable is a purely inline specialization of
 * integerNameTable which provides methods for strongly-typed enums
 * ("enum class").
 */

template<typename T>
class enumerationNameTable : public integerNameTable {
public:
	// Explicitly inherit our constructor
	// using integerNameTable::integerNameTable; // Doesn't work until g++-4.8, unfortunately
	enumerationNameTable(std::initializer_list<const char *> names)
	  : integerNameTable(names)
	{};
	

	// Explanation for other people who are (like me) not super
	// familiar with C++: Type-based function overloading only
	// works among functions in the same scope, and a subclass's
	// methods are in a different scope than its superclass'. So,
	// to keep the enum-specialized versions of get/valid from
	// hiding the unspecialized versions, we explicitly bring our
	// parent's versions of those methods into scope here.
	using integerNameTable::valid;

	bool get(const std::string& str, T *num) const {
		auto p = find(str);
		if (p == backward.cend()) {
			return false;
		} else {
			*num = static_cast<T>(p->second);
			return true;
		}
	};

	const std::string& get(T num) const {
		return integerNameTable::get(static_cast<int>(num));
	}

	bool valid(T num) const {
		return valid(static_cast<int>(num));
	}
};

