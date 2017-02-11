
all:  lib/libjansson.a 
.PHONY: all

base:=$(shell git rev-parse --show-toplevel)
tohere:=$(shell git rev-parse --show-prefix)

# Submodules

jansson/.git:
	( cd $(base) && git submodule update --init -- $(tohere)/jansson )

# Build jansson
lib/libjansson.a include/jansson.a: jansson/.git
	mkdir -p lib include
	( cd jansson && autoreconf -vi && \
	./configure --disable-shared --prefix "$(CURDIR)" \
        && make && make check && make install )

