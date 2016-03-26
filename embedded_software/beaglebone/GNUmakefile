# This file uses GNU make extensions.

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os

# Paths
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

BLACKLIB=submodules/BlackLib/v3_0

CC = gcc
CXX = g++

CFLAGS   += $(OPTS)
CXXFLAGS += $(OPTS) -std=gnu++11
CPPFLAGS +=-I submodules/include -I include -I submodules/smhasher/src -MMD -MT $@
LDFLAGS  +=-L submodules/lib

LDLIBS += -ljansson -lsqlite3 -lBlackLib -lm -lrt

export CC CXX OPTS

VPATH=src/common:src/nav:submodules/smhasher/src

all: submodules nav hb.fcgi master
.PHONY: all

MurmurHash3.o: 

LIBHACKERBOAT_SRCS= stateStructTypes.cpp 
LIBHACKERBOAT_SRCS+= logs.cpp 
LIBHACKERBOAT_SRCS+= sqliteStorage.cpp
LIBHACKERBOAT_SRCS+= location.cpp 
LIBHACKERBOAT_SRCS+= navigation.cpp 
LIBHACKERBOAT_SRCS+= waypoint.cpp 
LIBHACKERBOAT_SRCS+= gps.cpp 
LIBHACKERBOAT_SRCS+= arduinoState.cpp 
LIBHACKERBOAT_SRCS+= boneState.cpp
LIBHACKERBOAT_SRCS+= enumtable.cpp  
#LIBHACKERBOAT_SRCS+= MurmurHash3.cpp 
LIBHACKERBOAT_SRCS+= jansson_extras.cpp 
LIBHACKERBOAT_SRCS+= json_utilities.cpp 
LIBHACKERBOAT_SRCS+= json_ostream.cpp

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
TEST_OBJS=enumtable_test.o sqliteStorage_test.o smallJson_test.o boneState_test.o arduinoState_test.o navigation_test.o
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

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf *.a