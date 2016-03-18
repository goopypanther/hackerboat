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

OBJ=obj
VPATH=src/common:src/nav

all: nav hb.fcgi
.PHONY: all test

LIBHACKERBOAT_SRCS= stateStructTypes.cpp location.cpp navigation.cpp logs.cpp sqliteStorage.cpp waypoint.cpp gps.cpp enumtable.cpp arduinoState.cpp boneState.cpp MurmurHash3.cpp
libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o))

arduinoState.o: CPPFLAGS+= -I $(BLACKLIB)

NAV_SRCS=nav/hackerboatNavigator.cpp
nav: $(addprefix src/,$(NAV_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

FCGI_SRCS=cgi/hackerboatFastCGI.cpp
hb.fcgi:  $(addprefix src/,$(FCGI_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

# Unit test targets
storage_test: enumtable_test.o sqliteStorage_test.o gtest.o gtest_main.o libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
test: storage_test
	./storage_test


# gtest rules
gtest.o: $(GTEST_DIR) $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -I $(GTEST_DIR) -c -o $@ $(GTEST_DIR)/src/gtest-all.cc
gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -c -o $@ $^

$(OBJ)/:
	mkdir -p $(OBJ)

-include $(LIBHACKERBOAT_SRCS:.cpp=.d)
-include $(addprefix src/,$(NAV_SRCS:.cpp=.d))
