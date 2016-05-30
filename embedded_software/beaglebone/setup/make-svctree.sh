#!/bin/sh

#
# This creates a daemontools service directory for the hackerboat daemons.
# It's invoked by the beaglebone makefile for the 'svctree' target
# (and indirectly for the 'run' target).
#

if [ $# -ne 2 -o ! -d "$1" ]; then
    echo "usage: $0 srcdir destdir" 1>&2
    exit 1
fi

daemons="master hb.fcgi nav gpsParse"

for daemon in $daemons
do
    if [ ! -x "$1/$daemon" ]; then
	echo "$0: error: $1/$daemon does not exist or is not executable" 1>&2
	exit 1
    fi
done

mkdir -p "$2" || exit 1
for daemon in $daemons
do
    echo "Daemon: $daemon"
    if [ ! -d "$2/$daemon" ]; then
	mkdir "$2/$daemon" || exit 1
    else
	rm -f "$2/$daemon/run" || exit 1
    fi
    (
	echo "#!/bin/sh"
	echo -n "exec "
	case "$daemon" in
	    *.fcgi)
		echo -n 'spawn-fcgi -n -M 0666 -s `realpath "$PWD"`/fcgi.s -- '
		;;
	    *)
		;;
	esac
	realpath  "$1/$daemon" || exit 1
    ) > "$2/$daemon/run"
    chmod +x "$2/$daemon/run" || exit 1
    rm -f "$2/$daemon/$daemon"

done

echo "You can start these daemons by running \"svscan $2\""

exit 0
