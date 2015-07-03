/**
 * @file Nmea.c
 * @brief Interfaces with uBlox neo6m GPS module via UART & decodes NMEA stings
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version 1.0
 * @since Feb 1, 2015
 */

#include "includes.h"

#ifndef _STDLIB_H
#warning "stdlib.h required, including now. You should probably manually add stdlib.h to your includes."
#include <stdlib.h>
#endif

#ifndef _MATH_H
#warning "math.h required, including now. You should probably manually add math.h to your includes."
#include <math.h>
#endif

// Defines
#define NEO6M_UART_PUTS(x) uartGpsSend(x)
#define NEO6_UART_GETC() getchar()

#define NMEABUFFER_LENGTH 82
#define NMEAM_MINIMUM_SENTENCE_LENGTH 20

#define DEGREES_PER_MINUTE 0.0166666667f
#define MPS_PER_KNOT	   0.5144444444f

/**
 * Structure of pointers to important tokens within NMEA RMC string
 */
typedef struct nmea_ascii_pointers_t {
	uint8_t *start;
	uint8_t *time;
	uint8_t *status;
	uint8_t *lat;
	uint8_t *latSign;
	uint8_t *lon;
	uint8_t *lonSign;
	uint8_t *speed;
	uint8_t *course;
	uint8_t *date;
	uint8_t *checksum;
} nmea_ascii_pointers_t;

// Function prototypes
void Neo6mMeldDataISR(void);
void Neo6mInit(void);
void Neo6mParseBuffer(void);
nmea_time_t nmeaTimeParser(const uint8_t *date, const uint8_t *time);
nmea_space_t nmeaSpaceParser(const uint8_t *lat, const uint8_t *latSign, const uint8_t *lon, const uint8_t *lonSign);
nmea_velocity_t nmeaVelocityParser(const uint8_t *speed, const uint8_t *course);
nmea_rmc_t Neo6mGetData(void);
int32_t Neo6mGetLat(void);
int32_t Neo6mGetLon(void);
uint32_t Neo6mGetSpeed(void);
uint32_t Neo6mGetHeadingMadeGood(void);
uint32_t Neo6mGetStatus(void);

// Static variables
static volatile uint8_t nmeaBuffer[NMEABUFFER_LENGTH];
static volatile uint8_t nmeaString[NMEABUFFER_LENGTH];
static nmea_rmc_t nmeaData;

static const uint8_t *const nmeaInitCommands[] = {
	"$PUBX,40,GGA,0,0,0,0,0,0*5A\r\n",
	"$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n",
	"$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n",
	"$PUBX,40,GSV,0,0,0,0,0,0*59\r\n",
	"$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n",
};

/**
 * Neo6mInit
 */
void Neo6mInit(void) {
	uint32_t i;

	// Fill buffer with nulls
	for (i = 0; i < NMEABUFFER_LENGTH; i++) {
		nmeaBuffer[i] = 0x00;
		nmeaString[i] = 0x00;
	}

	// Send commands to neo6 to disable extra nmea sentences
	for (i = 0; i < 5; i++) {
		NEO6M_UART_PUTS(nmeaInitCommands[i]);
	}

	nmeaData.space.lat = 0;
	nmeaData.space.lon = 0;
	nmeaData.time.day = 0;
	nmeaData.time.month = 0;
	nmeaData.time.year = 0;
	nmeaData.time.hour = 0;
	nmeaData.time.minute = 0;
	nmeaData.time.second = 0;
	nmeaData.velocity.headingMadeGood = 0;
	nmeaData.velocity.speed = 0;
	nmeaData.status = FALSE;
}

/**
 * Neo6mMeldDataISR
 *
 * Uart 1 interrupt handler to add incoming serial bytes to the nmea buffer.
 * When \r\n or \n\r sequence is received, the nmea buffer is copied into the
 * nmea string buffer.
 */
void Neo6mMeldDataISR(void) {
	uint32_t i;

	// Shift nmeaBuffer by 1 byte
	for (i = 0; i < (NMEABUFFER_LENGTH - 1); i++) {
		nmeaBuffer[i] = nmeaBuffer[i + 1];
	}

	nmeaBuffer[81] = NEO6_UART_GETC(); // Store incoming byte into buffer

	// If last 2 characters were newline, complete string has been received
	if ((nmeaBuffer[81] == '\n') || (nmeaBuffer[81] == '\r')) {

		// Copy buffer into complete string
		for (i = 0; i < NMEABUFFER_LENGTH; i++) {
			nmeaString[i] = nmeaBuffer[i];
		}
	} else {}
}

/**
 * Neo6mParseBuffer
 *
 * Atomically copies nmea string buffer and finds start of nmea sentence.
 * Verifies sentence is RMC type and calculates checksum. Sentence is then
 * tokenized into appropriate fields. If checksum is valid, fields are passed
 * to time and space parsing functions.
 */
void Neo6mParseBuffer(void) {
	uint8_t currentNmeaString[NMEABUFFER_LENGTH];
	uint32_t i;
	uint32_t nmeaStringStart;
	uint32_t calculatedChecksum;
	uint32_t expectedChecksum;
	nmea_ascii_pointers_t pointers;
	nmea_rmc_t localNmeaData;

	// Make local copy of nmea string as atomic operation
	//UART1_DISABLE_IRQ();
	for (i = 0; i < NMEABUFFER_LENGTH; i++) {
		currentNmeaString[i] = nmeaString[i];
	}

	// Clear nmea string
	for (i = 0; i < (NMEABUFFER_LENGTH - 1); i++) {
		nmeaString[i] = 0x00;
	}
	//UART1_ENABLE_IRQ();

	// Seek to start of buffer ($)
	for (i = 0; ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != '$')); i++) {}

	// Verify nmea sentence is RMC type
	if ((i < (NMEABUFFER_LENGTH - NMEAM_MINIMUM_SENTENCE_LENGTH)) &&
		(currentNmeaString[i + 1] == 'G') &&
		(currentNmeaString[i + 2] == 'P') &&
		(currentNmeaString[i + 3] == 'R') &&
		(currentNmeaString[i + 4] == 'M') &&
		(currentNmeaString[i + 5] == 'C')) {

		i++; // Point first address character
		calculatedChecksum = 0x00;
		// Loop through every message character until terminating *
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != '*')) {
			calculatedChecksum ^= currentNmeaString[i]; // Compute checksum
			i++;
		}

		i = 0;

		// Seek to start of buffer ($)
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != '$')) {
			i++;
		}
		i++; // Step over delimiter
		pointers.start = &currentNmeaString[i]; // Store pointer to message id

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.time = &currentNmeaString[i]; // Store pointer to time string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.status = &currentNmeaString[i]; // Store pointer to status string

		i++; // Step over status char to delimiter
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.lat = &currentNmeaString[i]; // Store pointer to lat string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.latSign = &currentNmeaString[i]; // Store pointer to lat sign

		i++; // Step over lat sign char to delimiter
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.lon = &currentNmeaString[i]; // Store pointer to lon string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.lonSign = &currentNmeaString[i]; // Store pointer to lon sign

		i++; // Step over lon sign char to delimiter
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.speed = &currentNmeaString[i]; // Store pointer to speed string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.course = &currentNmeaString[i]; // Store pointer to course string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i++; // Step over delimiter
		pointers.date = &currentNmeaString[i]; // Store pointer to date string

		// Run through buffer until delimiter character
		while ((i < NMEABUFFER_LENGTH) && (currentNmeaString[i] != ',')) {
			i++;
		}
		currentNmeaString[i] = 0x00; // Convert to null string terminator
		i += 5; // Step over unused field delimiters (neo6 does not provide mag var info)
		pointers.checksum = &currentNmeaString[i]; // Store pointer to checksum string

		i += 2; // Step over checksum string
		currentNmeaString[i] = 0x00; // Convert to null string terminator

		// Convert string to hexadecimal value
		expectedChecksum = (uint32_t) strtol(pointers.checksum, 0x00, 16);

		// Compare checksums to verify nmea sentence is good
		if (calculatedChecksum == expectedChecksum) {
			// Decode status
			if (*(pointers.status) == 'A') {
				localNmeaData.status = TRUE;
			} else {
				localNmeaData.status = FALSE;
			}

			// Decode date & time
			localNmeaData.time = nmeaTimeParser(pointers.date, pointers.time);

			// Decode space
			localNmeaData.space = nmeaSpaceParser(pointers.lat, pointers.latSign, pointers.lon, pointers.lonSign);

			// Decode velocity
			localNmeaData.velocity = nmeaVelocityParser(pointers.speed, pointers.course);

			nmeaData = localNmeaData;
		} else {}
	} else {}
}

/**
 * nmeaTimeParser
 *
 * @param date pointer to unsigned 8 bit character string of nmea date information
 *
 * @param time pointer to unsigned 8 bit character string of nmea time information
 *
 * @return nmea time structure variable containing date and time information
 */
nmea_time_t nmeaTimeParser(const uint8_t *date, const uint8_t *time) {
	nmea_time_t nmeaTime;
	uint32_t temp;

	temp = (uint32_t) strtof(time, 0x00); // Convert string into decimal data

	nmeaTime.hour = temp / 10000;
	nmeaTime.minute = (temp / 100) % 100;
	nmeaTime.second = temp % 100;

	temp = (uint32_t) strtof(date, 0x00); // Convert string into decimal data

	nmeaTime.day = temp / 10000;
	nmeaTime.month = (temp / 100) % 100;
	nmeaTime.year = temp % 100;

	return (nmeaTime);
}

/**
 * nmeaSpaceParser
 *
 * @param lat pointer to unsigned 8 bit character string of nmea lat information
 *
 * @param latSign pointer to unsigned 8 bit character string of nmea n/s information
 *
 * @param lon pointer to unsigned 8 bit character string of nmea lon information
 *
 * @param lonSign pointer to unsigned 8 bit character string of nmea e/w information
 *
 * @return nmea space structure containing information about location
 */
nmea_space_t nmeaSpaceParser(const uint8_t *lat, const uint8_t *latSign, const uint8_t *lon, const uint8_t *lonSign) {
	nmea_space_t nmeaSpace;
	float_t degrees;
	float_t minutes;
	float_t temp;

	// Convert ddmm.mmmm format to dd.dddddd
	temp = strtof(lat, 0x00);
	minutes = fmodf(temp, 100);
	degrees = truncf(temp / 100);
	degrees += minutes * DEGREES_PER_MINUTE;
	// Make negative if southern hemisphere
	if (*latSign == 'S') {
		degrees *= -1;
	} else {}
	degrees *= 1000000;
	degrees = roundf(degrees);
	nmeaSpace.lat = (int32_t) degrees;

	// Convert ddmm.mmmm format to dd.dddddd
	temp = strtof(lon, 0x00);
	minutes = fmodf(temp, 100);
	degrees = truncf(temp / 100);
	degrees += minutes * DEGREES_PER_MINUTE;
	// Make negative if western hemisphere
	if (*lonSign == 'W') {
		degrees *= -1;
	} else {}
	degrees *= 1000000;
	degrees = roundf(degrees);
	nmeaSpace.lon = (int32_t) degrees;

	return (nmeaSpace);
}

/**
 * nmeaVelocityParser
 *
 * @param speed pointer to unsigned 8 bit character string of nmea speed information
 *
 * @param course pointer to unsigned 8 bit character string of nmea velocity information
 *
 * @return nmea velocity structure variable containing speed and heading information
 */
nmea_velocity_t nmeaVelocityParser(const uint8_t *speed, const uint8_t *course) {
	nmea_velocity_t nmeaVelocity;
	float_t temp;

	temp = strtof(speed, 0x00);
	temp *= MPS_PER_KNOT; // Convert to meter/second
	nmeaVelocity.speed = (uint32_t) roundf(temp);

	temp = strtof(course, 0x00);
	temp = roundf(temp);

	// Prevent wraparound ambiguity
	if (temp == 360) {
		temp = 0;
	} else {}

	nmeaVelocity.headingMadeGood = (uint32_t) temp;

	return (nmeaVelocity);
}

/**
 * Neo6mGetData
 *
 * @return structure of all current navigation data.
 * Status true if GPS has a fix
 * Space structure, fixed point 6 decimal precision (11cm) signed 32 bit
 * Time structure, unsigned 32 bit 24h UTC hour/min/sec, day/month/year 00-99
 * Velocity structure, unsigned 32 bit speed in m/s, heading in degrees 0-359
 */
nmea_rmc_t Neo6mGetData(void) {
	return (nmeaData);
}

/**
 * Neo6mGetLat
 *
 * @return fixed point 6 decimal (11cm) precision signed 32 bit lat
 */
int32_t Neo6mGetLat(void) {
	return (nmeaData.space.lat);
}

/**
 * Neo6mGetLon
 *
 * @return fixed point 6 decimal (11cm) precision signed 32 bit lon
 */
int32_t Neo6mGetLon(void) {
	return (nmeaData.space.lon);
}

/**
 * Neo6mGetSpeed
 *
 * @return unsigned 32 bit speed in m/s
 */
uint32_t Neo6mGetSpeed(void) {
	return (nmeaData.velocity.speed);
}

/**
 * Neo6mGetHeadingMadeGood
 *
 * @return unsigned 32 bit heading in degrees 0-359
 */
uint32_t Neo6mGetHeadingMadeGood(void) {
	return (nmeaData.velocity.headingMadeGood);
}

/**
 * Neo6mGetStatus
 *
 * @return True if GPS has a fix
 */
uint32_t Neo6mGetStatus(void) {
	return (nmeaData.status);
}
