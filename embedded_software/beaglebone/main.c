/**
 * @file main.c
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version
 * @since Jun 29, 2015
 */


#include "includes.h"

// Defines


// Function prototypes


// Static variables

int main (int argc, char* argv[]) {
	int returnState;
	nmea_rmc_t gpsCoords;

	uartInit("gps", "lowlevel");

	while (Neo6mGetStatus() == FALSE) {}

	gpsCoords = Neo6mGetData();

	printf("Lat: %u Lon: %u\nVel: %u\n", gpsCoords.space.lat, gpsCoords.space.lon, gpsCoords.velocity.speed);

	return (returnState);
}
