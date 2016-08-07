/******************************************************************************
 * Hackerboat RC input module
 * RCinput.hpp
 * This module reads incoming data over the SBUS
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef RCINPUT_H
#define RCINPUT_H

extern "C" {
#include <sqlite3.h>
#include <assert.h>
}

#include "enumdefs.hpp"
#include "enumtable.hpp"
 
 class rcInputClass {
	 public:
		rcInputClass (String path);
		int getThrottle (void);
		int getRudder (void);
		rcModeEnum getMode (void);
 };
 
#endif