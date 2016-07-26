#!/usr/bin/gawk -f

BEGIN {
	FS=","
}

$1 ~ /^[0-9]+$/ {print}
