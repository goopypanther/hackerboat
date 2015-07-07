/**
 * @file Map.h
 * @brief Computes directions and angles between lat/lon coordinates.
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version 1.0
 * @since Feb 3, 2015
 */

#ifndef MAP_H_
#define MAP_H_

#ifndef NEO6M_H_
/**
 * Fixed point 6 decimal (11cm) precision signed 32 bit lat & lon
 */
typedef struct nmea_space_t {
	int32_t lat;
	int32_t lon;
} nmea_space_t;
#endif

/**
 * Map navigation structure spatially relating two waypoints
 */
typedef struct map_navigation_t {
	uint32_t distance;
	uint32_t bearing;
} map_navigation_t;

extern uint32_t MapDistance(nmea_space_t a, nmea_space_t b);
extern uint32_t MapBearing(nmea_space_t a, nmea_space_t b);
extern map_navigation_t MapNavigation(nmea_space_t a, nmea_space_t b);

#endif /* MAP_H_ */
