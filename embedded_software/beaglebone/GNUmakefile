# This file uses GNU make extensions.

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os

# Paths
DEPSDIR=dependencies
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

BLACKLIB=$(DEPSDIR)/BlackLib/v3_0

# On the Beaglebone, the default compiler is gcc 4.6, which doesn't support the C++11 features we use (mostly, initializers for non-static data members). This can be any C++11-capable compiler.
CC = gcc-4.7
CXX = g++-4.7

CFLAGS   += $(OPTS)
CXXFLAGS += $(OPTS) -std=gnu++11
CPPFLAGS +=-I dependencies/include -I include -MMD -MT $@
LDFLAGS  +=-L dependencies/lib

LDLIBS += -ljansson -lsqlite3 -lm -lrt

LIBHACKERBOAT_SRCS=location.cpp navigation.cpp logs.cpp sqliteStorage.cpp waypoint.cpp gps.cpp enumtable.cpp

NAV_SRCS=nav/hackerboatNavigator.cpp

OBJ=obj
VPATH=src/common:src/nav

nav: $(addprefix src/,$(NAV_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)


libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o))

# gtest rules
gtest.o: $(GTEST_DIR) $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -I $(GTEST_DIR) -c -o $@ $(GTEST_DIR)/src/gtest-all.cc
gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -c -o $@ $^

$(OBJ)/:
	mkdir -p $(OBJ)

-include $(LIBHACKERBOAT_SRCS:.cpp=.d)
-include $(addprefix src/,$(NAV_SRCS:.cpp=.d))
