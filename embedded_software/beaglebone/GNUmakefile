# This file uses GNU make extensions.

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os

# Paths
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

BLACKLIB=submodules/BlackLib/v3_0
MINMEA=submodules/minmea

CFLAGS   += $(OPTS) -std=gnu11
CXXFLAGS += $(OPTS) -std=gnu++11
CPPFLAGS += -I submodules/include -I include -MMD -MT $@
CPPFLAGS += -I $(BLACKLIB)
CPPFLAGS += -I $(MINMEA)
LDFLAGS  += -L submodules/lib

LDLIBS += -ljansson -lsqlite3 -lBlackLib -lfcgi -lm -lrt

export OPTS

VPATH=src/common:src/nav:submodules/minmea

all: submodules nav hb.fcgi master gpsParse
test: storage_test
	./storage_test
.PHONY: all test

ifdef SVCTREE
svctree: submodules nav hb.fcgi master 
	./setup/make-svctree.sh . $(SVCTREE)
else
svctree:
	@echo "Error: You must define SVCTREE (e.g., \"$(MAKE) SVCTREE=/home/foo/svc $(MAKECMDGOALS)\")"
	@false
endif
run: svctree
	svscan $(SVCTREE)
.PHONY: svctree run

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
LIBHACKERBOAT_SRCS+= jansson_extras.cpp 
LIBHACKERBOAT_SRCS+= json_utilities.cpp 
LIBHACKERBOAT_SRCS+= json_ostream.cpp
LIBHACKERBOAT_SRCS+= minmea.cpp

libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o))
ALL_OBJS+=$(LIBHACKERBOAT_SRCS:.cpp=.o)

NAV_SRCS=nav/hackerboatNavigator.cpp
NAV_OBJS=$(addprefix src/,$(NAV_SRCS:.cpp=.o))
ALL_OBJS+=$(NAV_OBJS)
nav: $(NAV_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

FCGI_SRCS= cgi/hackerboatFastCGI.cpp cgi/RESTdispatch.cpp
src/cgi/RESTdispatch.o:  CPPFLAGS+= -I $(BLACKLIB)
FCGI_OBJS=$(addprefix src/,$(FCGI_SRCS:.cpp=.o))
ALL_OBJS+=$(FCGI_OBJS)
hb.fcgi: $(FCGI_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

MASTER_SRCS=master/hackerboatMasterControl.cpp master/stateMachine.cpp
MASTER_OBJS=$(addprefix src/,$(MASTER_SRCS:.cpp=.o))
ALL_OBJS+=$(MASTER_OBJS)
master: $(MASTER_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

GPS_SRCS= gpsParse/gpsReader.cpp
GPS_OBJS=$(addprefix src/,$(GPS_SRCS:.cpp=.o))
ALL_OBJS+=$(GPS_OBJS)
gpsParse: $(GPS_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	
# Unit test targets
TEST_OBJS=enumtable_test.o sqliteStorage_test.o smallJson_test.o boneState_test.o arduinoState_test.o navigation_test.o
GTEST_OBJS=test_utilities.o gtest.o gtest_main.o
ALL_OBJS+= $(TEST_OBJS) $(GTEST_OBJS)
storage_test: $(TEST_OBJS) $(GTEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

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

clean:
	rm -f libhackerboat.a
	rm -f $(ALL_OBJS)
	rm -f $(ALL_OBJS:.o=.d)
	rm -f master
	rm -f hb.fcgi
	
.PHONY: clean

-include $(ALL_OBJS:.o=.d)

