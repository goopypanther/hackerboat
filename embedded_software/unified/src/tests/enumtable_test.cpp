

#include "enumtable.hpp"
#include <stdexcept>
#include <gtest/gtest.h>

enum class silly : short { ALPHA, BETA, GAMMA, DELTA };

const EnumNameTable<silly> staticTable = { "Alpha", "Beta", "Gamma", "Delta" };


TEST(Serialization, Enumerations) {
	silly s(silly::GAMMA);

	// Test that we can look up some strings
	EXPECT_TRUE(staticTable.get("Alpha", &s));
	EXPECT_EQ(s, silly::ALPHA);
	
	EXPECT_TRUE(staticTable.get("Beta", &s));
	EXPECT_EQ(s, silly::BETA);

	EXPECT_TRUE(staticTable.get("Delta", &s));
	EXPECT_EQ(s, silly::DELTA);

	// And that we can't look up some strings we shouldn't
	EXPECT_FALSE(staticTable.get("Alph", &s));
	EXPECT_FALSE(staticTable.get("Alphas", &s));
	EXPECT_FALSE(staticTable.get("", &s));
	
	// Check the range of .valid()
	EXPECT_FALSE(staticTable.valid(-1));
	EXPECT_TRUE(staticTable.valid(0));
	EXPECT_TRUE(staticTable.valid(3));
	EXPECT_FALSE(staticTable.valid(4));

	// Test that we can look up enum names
	EXPECT_EQ(staticTable.get(silly::ALPHA), "Alpha");
	EXPECT_EQ(staticTable.get(silly::DELTA), "Delta");

	// And that an "impossible" enum value throws an exception
	EXPECT_THROW(staticTable.get(static_cast<silly>(-1)), std::out_of_range);
	EXPECT_THROW(staticTable.get(static_cast<silly>(4)), std::out_of_range);
}
