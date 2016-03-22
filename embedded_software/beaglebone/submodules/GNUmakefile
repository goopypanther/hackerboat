
all:  lib/libjansson.a lib/libBlackLib.a
.PHONY: all

base:=$(shell git rev-parse --show-toplevel)
tohere:=$(shell git rev-parse --show-prefix)

# Submodules
BlackLib/.git:
	( cd $(base) && git submodule update -- $(tohere)/BlackLib )
jansson/.git:
	( cd $(base) && git submodule update -- $(tohere)/jansson )

# Build jansson
lib/libjansson.a include/jansson.a: jansson/.git
	mkdir -p lib include
	( cd jansson && autoreconf -vi && \
	./configure --disable-shared --prefix "$(CURDIR)" \
        && make && make check && make install )

# Build BlackLib
# BlackLib doesn't currently have a makefile of its own, so we kludge one together here.
CFLAGS   += $(OPTS)
CXXFLAGS += $(OPTS) -std=gnu++11
BLACK_SRCS=BlackCore.cpp BlackUART/BlackUART.cpp BlackGPIO/BlackGPIO.cpp
BLACK_DIR=BlackLib/v3_0
CPPFLAGS+=-I$(BLACK_DIR)
lib/libBlackLib.a: BlackLib/.git lib/libBlackLib.a($(addprefix $(BLACK_DIR)/,$(BLACK_SRCS:.cpp=.o))) 

