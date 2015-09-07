# Hackerboat Navigator Makefile
APPNAME=hackerboatNavigator
SOURCES=args.c boatState.c command.c currentTime.c heartbeat.c log.c main.c Map.c mavlinkWrapper.c Neo6m.c param.c uart.c udp.c voltage.c
INCLUDES=./mavlink/hackerboat

COMPILER=gcc

all:
	$(COMPILER) -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm -lpthread
	du -h $(APPNAME)

debug:
	$(COMPILER) -g -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm -lpthread
	du -h $(APPNAME)

clean:
	rm $(APPNAME)

doxy:
	doxygen doxyfile
