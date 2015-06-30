# My makefile
APPNAME=hackerboatNavigator
SOURCES=main.c args.c log.c udp.c mavlinkWrapper.c Neo6m.c
INCLUDES=./mavlink/hackerboat

COMPILER=gcc

all:
	$(COMPILER) -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm
	du -h $(APPNAME)

debug:
	$(COMPILER) -g -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm
	du -h $(APPNAME)

clean:
	rm $(APPNAME)

doxy:
	doxygen doxyfile
