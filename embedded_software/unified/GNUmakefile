# This file uses GNU make extensions.

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os -Wall
# OPTS = -gdwarf-4 -g3 -Wall

# Paths
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

MINMEA=submodules/minmea

CFLAGS   += $(OPTS) -std=gnu11 
CXXFLAGS += $(OPTS) -Wno-reorder -std=gnu++11 -pthread
CPPFLAGS += -I submodules/include -I include -MMD -MT $@
LDFLAGS  += -L submodules/lib

LDLIBS += -ljansson -lsqlite3 -lm -lrt

export OPTS

VPATH=src/common:src/master
all: submodules master
test: storage_test
	./storage_test
.PHONY: all test

ifdef SVCTREE
svctree: submodules master 
	./setup/make-svctree.sh . $(SVCTREE)
else
svctree:
	$(error You must define SVCTREE (e.g., "$(MAKE) SVCTREE=/home/foo/svc $(MAKECMDGOALS)"))
endif
run: svctree
	svscan $(SVCTREE)
.PHONY: svctree run

LIBHACKERBOAT_SRCS= logs.cpp 
LIBHACKERBOAT_SRCS+= sqliteStorage.cpp
LIBHACKERBOAT_SRCS+= location.cpp 
#LIBHACKERBOAT_SRCS+= gps.cpp 
LIBHACKERBOAT_SRCS+= enumtable.cpp  
LIBHACKERBOAT_SRCS+= jansson_extras.cpp 
LIBHACKERBOAT_SRCS+= json_utilities.cpp 
LIBHACKERBOAT_SRCS+= json_ostream.cpp

libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o))
ALL_OBJS+=$(LIBHACKERBOAT_SRCS:.cpp=.o)

MASTER_SRCS=master/hackerboatMasterControl.cpp 
#MASTER_SRCS+=master/stateMachine.cpp
MASTER_OBJS=$(addprefix src/,$(MASTER_SRCS:.cpp=.o))
ALL_OBJS+=$(MASTER_OBJS)
master: $(MASTER_OBJS) libhackerboat.a
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
	rm -f master nav hb.fcgi gpsParse

.PHONY: clean

-include $(ALL_OBJS:.o=.d)
