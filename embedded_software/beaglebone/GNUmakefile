# This file uses GNU make extensions.

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os

# Paths
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

BLACKLIB=submodules/BlackLib/v3_0

# On the Beaglebone, the default compiler is gcc 4.6, which doesn't support the C++11 features we use (mostly, initializers for non-static data members). This can be any C++11-capable compiler.
CC = gcc-4.7
CXX = g++-4.7

CFLAGS   += $(OPTS)
CXXFLAGS += $(OPTS) -std=gnu++11
CPPFLAGS +=-I submodules/include -I include -MMD -MT $@
LDFLAGS  +=-L submodules/lib

LDLIBS += -ljansson -lsqlite3 -lBlackLib -lm -lrt

export CC CXX OPTS

VPATH=src/common:src/nav

all: submodules nav hb.fcgi master
.PHONY: all

LIBHACKERBOAT_SRCS= stateStructTypes.cpp logs.cpp sqliteStorage.cpp
LIBHACKERBOAT_SRCS+= location.cpp navigation.cpp waypoint.cpp gps.cpp arduinoState.cpp boneState.cpp
LIBHACKERBOAT_SRCS+= enumtable.cpp  MurmurHash3.cpp jansson_extras.cpp json_utilities.cpp json_ostream.cpp
libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o))

arduinoState.o: CPPFLAGS+= -I $(BLACKLIB)

NAV_SRCS=nav/hackerboatNavigator.cpp
nav: $(addprefix src/,$(NAV_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

FCGI_SRCS=cgi/hackerboatFastCGI.cpp
hb.fcgi:  $(addprefix src/,$(FCGI_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

MASTER_SRCS=master/hackerboatMasterControl.cpp master/stateMachine.cpp
master: $(addprefix src/,$(MASTER_SRCS:.cpp=.o)) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

# Unit test targets
TEST_OBJS=enumtable_test.o sqliteStorage_test.o smallJson_test.o boneState_test.o arduinoState_test.o
GTEST_OBJS=test_utilities.o gtest.o gtest_main.o
storage_test: $(TEST_OBJS) $(GTEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
test: storage_test
	./storage_test
.PHONY: test

# gtest rules:
# gtest isn't installed as a library by the debian package because it
# depends on compiler flags. Instead, the recommended way to use it is
# to compile its pieces into object files ourselves and then link
# those in.
gtest.o: $(GTEST_DIR) $(GTEST_INC)
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -I $(GTEST_DIR) -c -o $@ $(GTEST_DIR)/src/gtest-all.cc
gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) -I $(GTEST_INC) -c -o $@ $^


submodules:
	@$(MAKE) -C submodules
.PHONY: submodules

-include $(LIBHACKERBOAT_SRCS:.cpp=.d)
-include $(addprefix src/,$(NAV_SRCS:.cpp=.d))
