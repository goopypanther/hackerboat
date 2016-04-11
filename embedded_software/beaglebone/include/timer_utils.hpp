/******************************************************************************
 * Hackerboat Beaglebone time utilities
 * timer_utils.hpp
 * functions for adding and subtracting timespec values
 * Written by Pierce Nichols, Apr 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void norm_timespec (timespec* a) {
	while (a->tv_nsec >= 1e9) {
		a->tv_sec++;
		a->tv_nsec -= 1e9;
	}
	while (a->tv_nsec < 0) {
		a->tv_sec--;
		a->tv_nsec += 1e9;
	}
}

void add_timespec (timespec* a, timespec* b, timespec* result) {
	int64_t ns_sum; 
	
	result->tv_sec = a->tv_sec + b->tv_sec;
	result->tv_nsec = a->tv_nsec + b->tv_nsec;
	norm_timespec(result);
}

bool subtract_timespec (timespec* a, timespec* b, timespec* result) {
	if (b->tv_sec > a->tv_sec) return false;	// This would result in a negative timespec, which is clearly forbidden by spec
	if ((b->tv_sec == a->tv_sec) &&				// This would also result in a negative timespec
		(b->tv_nsec > a->tv_nsec)) return false;
	result->tv_sec = a->tv_sec - b->tv_sec;
	result->tv_nsec = a->tv_nsec - b->tv_nsec;
	norm_timespec(result);
	
	return true;
}
