# My makefile
APPNAME=mavlink_udp
SOURCES=mavlink_udp.c
INCLUDES=./mavlink/hackerboat

COMPILER=gcc

all:
	$(COMPILER) -g -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm
	du -h $(APPNAME)

debug:
	$(COMPILER) -g -I $(INCLUDES) -o $(APPNAME) $(SOURCES) -lm
	du -h $(APPNAME)

clean:
	rm $(APPNAME)

doxy:
	doxygen doxyfile
