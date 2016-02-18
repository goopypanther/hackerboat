# This file uses GNU make extensions.

CPPFLAGS=-I dependencies/include -I include
LDFLAGS=-L dependencies/lib


NAV_SRCS=nav/hackerboatNavigator.cpp


nav: $(addprefix src/,$(NAV_SRCS:.cpp=.o))
	$(CC) -o $@ $^ $(LDFLAGS)  $(LDLIBS)
