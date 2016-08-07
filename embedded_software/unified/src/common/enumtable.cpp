/******************************************************************************
 * Enumeration name table
 * 
 * A utility class for holding a statically-defined bidirectional
 * integer/string mapping.
 *
 * Written by Wim Lewis, Mar 2016
 *
 ******************************************************************************/

#include "enumtable.hpp"

using namespace std;

/* We jump through some hoops here so that we can ensure all the
 * members of integerNameTable are const, and that they share
 * their std::string reps. The pair<> constructor gets inlined
 * into the initializer_list constructor, which, combined with RNVO,
 * eliminates a bunch of unnecessary temporary allocations when
 * constructing a integerNameTable.
 */

static const pair<vector<string>, unordered_map<string, int> > computeHashes(initializer_list<const char *> names)
{
	pair <vector<string>, unordered_map<string, int> > result;

	result.first.reserve(names.size());
	for (const char *name : names) {
		result.first.emplace_back(name);
	}

	result.second.reserve(names.size());
	for(typeof(names.size()) i = 0; i < names.size(); i++) {
		result.second.emplace(result.first[i], i);
	}

	return result;
}

/* The initializer_list constructor is *not* inline, because we want
 * only one copy of it and everything it pulls in.  
 */

integerNameTable::integerNameTable(std::initializer_list<const char *> names)
	: integerNameTable(computeHashes(names))
{
};

bool integerNameTable::valid(int num) const
{
	return num >= 0 && (unsigned)num < forward.size();
}

const std::string& integerNameTable::get(int num) const
{
	return forward.at(num);
}

const std::unordered_map<std::string, int>::const_iterator integerNameTable::find(const std::string& str) const
{
	return backward.find(str);
}
