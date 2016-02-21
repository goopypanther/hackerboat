#!/bin/sh

if [ -n "$DEPSDIR" -a -d "$DEPSDIR" ]; then
    :
else
    echo "Must set DEPSDIR to an existing directory before running this script." >&2
    exit 1
fi

set -x
set -e

cd "$DEPSDIR"

if [ ! -d BlackLib ]; then
    git clone -n https://github.com/logos-electromechanical/BlackLib.git
    ( cd BlackLib && git checkout )
fi

if [ ! -d jansson ]; then
    git clone -n https://github.com/akheron/jansson
    ( cd jansson  && git checkout )
fi

mkdir -p lib include

( cd jansson \
        && autoreconf -vi \
        && ./configure --disable-shared --prefix "$DEPSDIR" \
        && make \
        && make check \
        && make install )

