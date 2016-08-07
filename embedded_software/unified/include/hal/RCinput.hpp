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

#include <string>
#include <stdlib.h>
#include <time.h>
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "hal/config.h"
 
 class rcInputClass {
	 public:
		rcInputClass (String devpath);	/**< Create a rcInput reader attached to serial port devpath 		*/
		rcInputClass (void);			/**< Create a rcInput reader attached to serial port defined in hal/config.h */
		int getThrottle (void);			/**< Get the last throttle position from the RC input 				*/
		int getRudder (void);			/**< Get the last rudder position from the RC input 				*/
		rcModeEnum getMode (void);		/**< Get the last mode command from the RC input 					*/	
		bool execute (void);			/**< Gather input from the RC S.BUS (meant to be called regularly)	*/
		~rcInputClass();				/**< Explicit destructor to make sure we close out the serial port.	*/
		
		timespec lastInput;				/**< Time that last input was processed 							*/
 };
 
#endif