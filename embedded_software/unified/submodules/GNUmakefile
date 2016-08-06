
all:  lib/libjansson.a include/minmea.h
.PHONY: all

base:=$(shell git rev-parse --show-toplevel)
tohere:=$(shell git rev-parse --show-prefix)

# Submodules

jansson/.git:
	( cd $(base) && git submodule update --init -- $(tohere)/jansson )
minmea/.git:
	( cd $(base) && git submodule update --init -- $(tohere)/minmea )

# Build jansson
lib/libjansson.a include/jansson.a: jansson/.git
	mkdir -p lib include
	( cd jansson && autoreconf -vi && \
	./configure --disable-shared --prefix "$(CURDIR)" \
        && make && make check && make install )

# Make minmea linkable
include/minmea.h: minmea/.git minmea/minmea.h
	cp minmea/minmea.h include/minmea.h
