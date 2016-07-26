#!/usr/bin/gawk -f
BEGIN {
	FS=",";
	print "Date,Time,Lat,Lon,Course Made Good,Speed Made Good (kts)";
}

/^\$GPRMC/ {
	startFlag = 1;
	UTCraw = $2;
	UTChrs = int(UTCraw/10000);
	UTCmins = int((UTCraw/100) - (UTChrs*100));
	UTCsec = int(UTCraw - ((UTChrs*10000) + (UTCmins*100)));
	latRaw = $4;
	latSign = $5;
	lonRaw = $6;
	lonSign = $7;
	latDeg = int(latRaw/100);
	latDec = ((latRaw - (latDeg * 100))/60);
	latOut = (latDeg + latDec);
	if (latSign == "S") {latOut = -latOut;}
	lonDeg = int(lonRaw/100);
	lonDec = ((lonRaw - (lonDeg * 100))/60);
	lonOut = (lonDeg + lonDec);
	if (lonSign == "W") {lonOut = -lonOut;}
	speed = $8;
	if (speed > maxSpeed) maxSpeed = speed;
	course = $9;
	DateRaw = $10;
	DateDay = int(DateRaw/10000);
	DateMon = int((DateRaw/100) - (DateDay*100));
	DateYear = 2000 + int(DateRaw - ((DateDay*10000) + (DateMon*100)));
	printf "%d/%d/%d,%d:%d:%d,", DateMon, DateDay, DateYear, UTChrs, UTCmins, UTCsec;
	printf "%8.5f,%8.5f,%4.2f,%3.2f\n", latOut, lonOut, course, speed;
}

END {
	printf "Max speed is %3.2f knots\n", maxSpeed;
}