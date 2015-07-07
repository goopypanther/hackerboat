/**
 * @file currentTime.h
 * @brief Provide current system time from GPS or internal clock
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 2, 2015
 */

#ifndef CURRENTTIME_H_
#define CURRENTTIME_H_

extern void currentTimeGet(nmea_time_t *currentTime);
extern uint64_t currentTimeMsSinceEpoch(void);

#endif /* CURRENTTIME_H_ */
