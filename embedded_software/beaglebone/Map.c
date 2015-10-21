/**
 * @file Map.c
 * @brief Computes directions and angles between lat/lon coordinates.
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version 1.0
 * @since Feb 3, 2015
 */

#include "includes.h"

#ifndef _MATH_H
#warning "math.h required, including now. You should probably manually add math.h to your includes."
#include <math.h>
#endif

// Defines
#define PI 3.14159265358979323846264338327f
#define EARTH_RADIUS_M 6371000.0f

// Function prototypes
uint32_t MapDistance(nmea_space_t a, nmea_space_t b);
uint32_t MapBearing(nmea_space_t a, nmea_space_t b);
map_navigation_t MapNavigation(nmea_space_t a, nmea_space_t b);

/**
 * MapDistance
 *
 * @param a nmea space struct lat & lon, current position
 *
 * @param b nmea space struct lat & lon, target position
 *
 * @return unsigned 32 bit distance in meters between a & b
 */
uint32_t MapDistance(nmea_space_t a, nmea_space_t b) {
	uint32_t distance;
	float_t targetLatRad;
	float_t targetLonRad;
	float_t currentLatRad;
	float_t currentLonRad;
	float_t tempx;
	float_t tempy;

	// Convert degrees to radians
	currentLatRad = ((float_t) a.lat) / 1000000 / (180 / PI);
	currentLonRad = ((float_t) a.lon) / 1000000 / (180 / PI);
	targetLatRad  = ((float_t) b.lat) / 1000000 / (180 / PI);
	targetLonRad  = ((float_t) b.lon) / 1000000 / (180 / PI);

	tempx = sinf((targetLatRad - currentLatRad) / 2);
	tempx = powf(tempx, 2);

	tempy = sinf((targetLonRad - currentLonRad) / 2);
	tempy = powf(tempy, 2);

	tempy = tempx + (tempy * cosf(currentLatRad) * cosf(targetLatRad));

	tempx = sqrtf(1 - tempy);
	tempy = sqrtf(tempy);

	tempy = 2 * atan2f(tempy, tempx);
	tempy *= EARTH_RADIUS_M;
	tempy = roundf(tempy);

	distance = (uint32_t) tempy;

	return (distance);
}

/**
 * MapBearing
 *
 * @param a nmea space struct lat & lon, current position
 *
 * @param b nmea space struct lat & lon, target position
 *
 * @return unsigned 32 bit degrees to target from current position, 0-359
 *
 */
uint32_t MapBearing(nmea_space_t a, nmea_space_t b) {
	uint32_t bearing;
	float_t targetLatRad;
	float_t targetLonRad;
	float_t currentLatRad;
	float_t currentLonRad;
	float_t tempx;
	float_t tempy;

	// Convert degrees to radians
	currentLatRad = ((float_t) a.lat) / 1000000 / (180 / PI);
	currentLonRad = ((float_t) a.lon) / 1000000 / (180 / PI);
	targetLatRad  = ((float_t) b.lat) / 1000000 / (180 / PI);
	targetLonRad  = ((float_t) b.lon) / 1000000 / (180 / PI);

	tempy = sinf(targetLonRad - currentLonRad) * cosf(targetLatRad);

	tempx = cosf(currentLatRad) * sinf(targetLatRad) -
			sinf(currentLatRad) * cosf(targetLatRad) *
			cosf(targetLonRad - currentLonRad);

	tempx = atan2f(tempy, tempx) * (180/PI);
	tempx = roundf(tempx);
	tempx = fmodf((tempx + 360), 360);

	bearing = (uint32_t) tempx;

	return (bearing);
}

/**
 * MapNavigation
 *
 * @param a nmea space struct lat & lon, current position
 *
 * @param b nmea space struct lat & lon, target position
 *
 * @return nav struct containing unsigned 32 bit bearing & distance from
 * current to target position.
 */
map_navigation_t MapNavigation(nmea_space_t a, nmea_space_t b) {
	map_navigation_t nav;

	nav.bearing = MapBearing(a, b);
	nav.distance = MapDistance(a, b);

	return (nav);
}
