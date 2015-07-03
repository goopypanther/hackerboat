/**
 * @file currentTime.c
 * @brief Provide current system time from GPS or internal clock
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 2, 2015
 */


#include "includes.h"

// Defines


// Function prototypes
void currentTimeGet(nmea_time_t *currentTime);

// Static variables

/**
 * Return current time from GPS or system clock if no GPS lock
 *
 * @param time pointer to \c nmea_time_t struct where result will be copied
 */
void currentTimeGet(nmea_time_t *currentTime) {
	nmea_rmc_t gpsData;
	time_t epochTime;
	struct tm *systemTime;

	gpsData = Neo6mGetData(); // Get most recent GPS data

	// Check if GPS has a fix
	if (gpsData.status == TRUE) {
		*currentTime = gpsData.time; // Transfer GPS time data

	} else {
	// If no GPS fix get current time from system (may be inaccurate)

		time(&epochTime); // Get ms since epoch
		systemTime = localtime(&epochTime); // Convert to time units

		currentTime->year = (uint32_t) (systemTime->tm_year + 1900);
		currentTime->month = (uint32_t) (systemTime->tm_mon + 1);
		currentTime->day = (uint32_t) systemTime->tm_mday;
		currentTime->hour = (uint32_t) systemTime->tm_hour;
		currentTime->minute = (uint32_t) systemTime->tm_min;
		currentTime->second = (uint32_t) systemTime->tm_sec;
	}
}
