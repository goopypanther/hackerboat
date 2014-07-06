#!/bin/sh

#define _POSIX_SOURCE
#define __USE_XOPEN2KXSI
#define __USE_XOPEN

gcc -O2 -I"./include/" -D_POSIX_SOURCE -D__USE_XOPEN2KXSI -D__USE_XOPEN -Wall -std=c99 stalkerLink.c -o stalkerLink

