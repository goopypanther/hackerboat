/**
 * @file Nmea.h
 * @brief Interfaces with uBlox neo6m GPS module via UART & decodes NMEA stings
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version 1.0
 * @since Feb 1, 2015
 */

#ifndef NEO6M_H_
#define NEO6M_H_

/**
 * Time structure of unsigned 32 bit values, 24h UTC time, year 00-99
 */
typedef struct nmea_time_t {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} nmea_time_t;

/**
 * Space structure of fixed point 6 decimal (11cm) precision
 * signed 32 bit lat & lon
 */
typedef struct nmea_space_t {
	int32_t lat;
	int32_t lon;
} nmea_space_t;

/**
 * Velocity structure of unsigned 32 bit values
 */
typedef struct nmea_velocity_t {
	uint32_t speed; /**< speed in m/s */
	uint32_t headingMadeGood; /**< heading 0-365 degrees */
} nmea_velocity_t;

/**
 * RMC NMEA string structure of structures
 */
typedef struct nmea_rmc_t {
	uint32_t status;
	nmea_time_t time;
	nmea_space_t space;
	nmea_velocity_t velocity;
} nmea_rmc_t;

extern void Neo6mMeldDataISR(void);
extern void Neo6mInit(void);
extern void Neo6mParseBuffer(void);
extern nmea_rmc_t Neo6mGetData(void);
extern int32_t Neo6mGetLat(void);
extern int32_t Neo6mGetLon(void);
extern uint32_t Neo6mGetSpeed(void);
extern uint32_t Neo6mGetHeadingMadeGood(void);
extern uint32_t Neo6mGetStatus(void);

#endif /* NEO6M_H_ */
