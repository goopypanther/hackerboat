#!/usr/bin/gawk -f
BEGIN {
	FS=","
	startFlag = 0;
	printf "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	printf "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
	printf "<Document>\n";
	printf "<Style id=\"yellowLineGreenPoly\">\n";
	printf "<LineStyle>\n";
	printf "<color>7f00ffff</color>\n";
	printf "<width>4</width>\n";
	printf "</LineStyle>\n";
	printf "<PolyStyle>\n";
	printf "<color>7f00ff00</color>\n";
	printf "</PolyStyle>\n";
	printf "</Style>\n";
}

/^\$GPRMC/ && !startFlag {
	startFlag = 1;
	UTCraw = $2;
	UTChrs = int(UTCraw/10000);
	UTCmins = int((UTCraw/100) - (UTChrs*100));
	UTCsec = int(UTCraw - ((UTChrs*10000) + (UTCmins*100)));
	DateRaw = $10;
	DateDay = int(DateRaw/10000);
	DateMon = int((DateRaw/100) - (DateDay*100));
	DateYear = 2000 + int(DateRaw - ((DateDay*10000) + (DateMon*100)));
	printf "<name>Hackerboat Test %d/%d/%d %d:%d UTC</name>\n", DateMon, DateDay, DateYear, UTChrs, UTCmins;
	printf "<description>Hackerboat test conducted on starting at %d/%d/%d %d:%d:%d UTC</description>/n", DateMon, DateDay, DateYear, UTChrs, UTCmins, UTCsec;
	printf "<Placemark>\n";
	printf "<name>Test Path</name>\n";
	printf "<styleUrl>#yellowLineGreenPoly</styleUrl>\n";
	printf "<LineString>\n";
	printf "<tessellate>1</tessellate>\n";
	printf "<altitudeMode>absolute</altitudeMode>\n";
	printf "<coordinates>\n";
}

/^\$GPGGA/ {
	latRaw = $3;
	latSign = $4;
	lonRaw = $5;
	lonSign = $6;
	latDeg = int(latRaw/100);
	latDec = ((latRaw - (latDeg * 100))/60);
	latOut = (latDeg + latDec);
	if (latSign == "S") {latOut = -latOut;}
	lonDeg = int(lonRaw/100);
	lonDec = ((lonRaw - (lonDeg * 100))/60);
	lonOut = (lonDeg + lonDec);
	if (lonSign == "W") {lonOut = -lonOut;}
	if (startFlag > 0) {printf "%8.5f, %8.5f,0\n", lonOut, latOut;}
}

END {
	printf "</coordinates>\n";
	printf "</LineString>\n";
	printf "</Placemark>\n";
	printf "</Document>\n";
	printf "</kml>\n";
}