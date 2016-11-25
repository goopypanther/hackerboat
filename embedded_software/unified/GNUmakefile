# This file uses GNU make extensions.

# use g++-6
CXX = /usr/bin/g++-6
CC = /usr/bin/gcc-6

# Tweakable / overridable compilation options (debugging and optimization)
OPTS = -gdwarf-4 -g3 -Os -Wall
#OPTS = -gdwarf-4 -g3 -Wall

# Paths
GTEST_DIR=/usr/src/gtest
GTEST_INC=/usr/include/gtest

CFLAGS   += $(OPTS) -std=gnu11 -I submodules/lsquaredc

CXXFLAGS += $(OPTS) -Wno-reorder -std=gnu++14 -pthread 

CPPFLAGS += -I submodules/include 
CPPFLAGS += -I submodules/lsquaredc
CPPFLAGS += -I include 
CPPFLAGS += -I submodules/PixelBone 
CPPFLAGS += -I submodules/hackerboat-private 
CPPFLAGS += -I submodules/date
CPPFLAGS += -I submodules/pstreams
CPPFLAGS += -D _USE_MATH_DEFINES
CPPFLAGS += -MMD -MT $@
CPPFLAGS += -fno-inline-small-functions

LDFLAGS  += -L /usr/local/lib

LDLIBS += -ljansson
LDLIBS += -lsqlite3
LDLIBS += -lm
LDLIBS += -lrt
LDLIBS += /usr/local/lib/libGeographic.a

export OPTS

VPATH=src/common:src/master:src/drivers:src/tests:submodules/lsquaredc
all: submodules master rcctrl
test: unit_tests
	./unit_tests
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
LIBHACKERBOAT_SRCS+= autoModes.cpp 
LIBHACKERBOAT_SRCS+= mqtt.cpp
LIBHACKERBOAT_SRCS+= waypoint.cpp
LIBHACKERBOAT_SRCS+= gpsdInput.cpp
LIBHACKERBOAT_SRCS+= rcModes.cpp
LIBHACKERBOAT_SRCS+= RCinput.cpp
LIBHACKERBOAT_SRCS+= adcInput.cpp
LIBHACKERBOAT_SRCS+= throttle.cpp
LIBHACKERBOAT_SRCS+= relay.cpp
LIBHACKERBOAT_SRCS+= lsm303.cpp
LIBHACKERBOAT_SRCS+= adc128d818.cpp
LIBHACKERBOAT_SRCS+= sqliteStorage.cpp
LIBHACKERBOAT_SRCS+= location.cpp 
LIBHACKERBOAT_SRCS+= gps.cpp 
LIBHACKERBOAT_SRCS+= enumtable.cpp  
LIBHACKERBOAT_SRCS+= ais.cpp   
LIBHACKERBOAT_SRCS+= pid.cpp   
LIBHACKERBOAT_SRCS+= twovector.cpp
LIBHACKERBOAT_SRCS+= orientation.cpp
LIBHACKERBOAT_SRCS+= orientationInput.cpp
LIBHACKERBOAT_SRCS+= jansson_extras.cpp 
LIBHACKERBOAT_SRCS+= json_utilities.cpp 
LIBHACKERBOAT_SRCS+= json_ostream.cpp
LIBHACKERBOAT_SRCS+= gpio.cpp
LIBHACKERBOAT_SRCS+= servo.cpp
LIBHACKERBOAT_SRCS+= boatState.cpp
LIBHACKERBOAT_SRCS+= boatModes.cpp
LIBHACKERBOAT_SRCS+= navModes.cpp
LIBHACKERBOAT_SRCS+= healthMonitor.cpp
LIBHACKERBOAT_C_SRCS+= lsquaredc.c

libhackerboat.a: libhackerboat.a($(LIBHACKERBOAT_SRCS:.cpp=.o) $(LIBHACKERBOAT_C_SRCS:.c=.o))
ALL_OBJS+=$(LIBHACKERBOAT_SRCS:.cpp=.o)
ALL_OBJS+=$(LIBHACKERBOAT_C_SRCS:.c=.o)

MASTER_SRCS=master/hackerboatMasterControl.cpp 
MASTER_OBJS=$(addprefix src/,$(MASTER_SRCS:.cpp=.o))
ALL_OBJS+=$(MASTER_OBJS)
master: $(MASTER_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

RC_SRCS=master/hackerboatRC.cpp 
RC_OBJS=$(addprefix src/,$(RC_SRCS:.cpp=.o))
rcctrl: $(RC_OBJS) libhackerboat.a 
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	
# Unit test targets
TEST_OBJS = enumtable_test.o 
TEST_OBJS += twovector_test.o 
TEST_OBJS += location_test.o 
TEST_OBJS += orientation_test.o 
TEST_OBJS += waypoint_test.o 
TEST_OBJS += pid_test.o 
TEST_OBJS += navmode_test.o 
TEST_OBJS += rcmode_test.o
TEST_OBJS += automode_test.o
TEST_OBJS += boatstate_test.o 
TEST_OBJS += boatmode_test.o 
#TEST_OBJS += sqliteStorage_test.o 
#TEST_OBJS += smallJson_test.o 
GTEST_OBJS=test_utilities.o gtest.o gtest_main.o
ALL_OBJS+= $(TEST_OBJS) $(GTEST_OBJS)
unit_tests: $(TEST_OBJS) $(GTEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

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

# Functional tests
RC_TEST_SRCS = functional_tests/rc_test.cpp
ORIENTATION_TEST_SRCS += functional_tests/orientation_test.cpp
ADC_TEST_SRCS += functional_tests/adc_test.cpp
RELAY_TEST_SRCS += functional_tests/relay_test.cpp
THROTTLE_TEST_SRCS += functional_tests/throttle_test.cpp
SERVO_TEST_SRCS += functional_tests/servo_test.cpp
GPS_TEST_SRCS += functional_tests/gps_test.cpp
RC_TEST_OBJS = $(addprefix src/,$(RC_TEST_SRCS:.cpp=.o))
ORIENTATION_TEST_OBJS = $(addprefix src/,$(ORIENTATION_TEST_SRCS:.cpp=.o))
ADC_TEST_OBJS = $(addprefix src/,$(ADC_TEST_SRCS:.cpp=.o))
RELAY_TEST_OBJS = $(addprefix src/,$(RELAY_TEST_SRCS:.cpp=.o))
THROTTLE_TEST_OBJS += $(addprefix src/,$(THROTTLE_TEST_SRCS:.cpp=.o))
SERVO_TEST_OBJS += $(addprefix src/,$(SERVO_TEST_SRCS:.cpp=.o))
GPS_TEST_OBJ += $(addprefix src/,$(GPS_TEST_SRCS:.cpp=.o))
ALL_OBJS+=$(RC_TEST_OBJS)
ALL_OBJS+=$(ORIENTATION_TEST_OBJS)

rc_test: $(RC_TEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

orientation_test: $(ORIENTATION_TEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

adc_test: $(ADC_TEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	
relay_test: $(RELAY_TEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

throttle_test: $(THROTTLE_TEST_OBJS) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

servo_test: $(SERVO_TEST_OBJS) libhackerboat.a 
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
	
gps_test: $(GPS_TEST_OBJ) libhackerboat.a
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

functional_tests: rc_test orientation_test adc_test relay_test throttle_test servo_test gps_test
	
clean:
	rm -f libhackerboat.a
	rm -f storage_test
	rm -f $(ALL_OBJS)
	rm -f $(ALL_OBJS:.o=.d)
	rm -f master nav hb.fcgi gpsParse

.PHONY: clean

-include $(ALL_OBJS:.o=.d)
