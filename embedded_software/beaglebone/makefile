# My makefile
APPNAME=mavlink_udp
SOURCES=mavlink_udp.c
INCLUDES=./mavlink/hackerboat

COMPILER=gcc

all:
	$(COMPILER) -I $(INCLUDES) -o $(APPNAME) $(SOURCES)
	du -h $(APPNAME)

debug:
	$(COMPILER) -g -I $(INCLUDES) -o $(APPNAME) $(SOURCES)
	du -h $(APPNAME)

clean:
	rm $(APPNAME)
