# This file uses GNU make extensions.

CXXFLAGS += -std=gnu++11
CPPFLAGS +=-I dependencies/include -I include
LDFLAGS  +=-L dependencies/lib


NAV_SRCS=nav/hackerboatNavigator.cpp


nav: $(addprefix src/,$(NAV_SRCS:.cpp=.o))
	$(CC) -o $@ $^ $(LDFLAGS)  $(LDLIBS)
