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
#include <atomic>
#include <thread>
#include <chrono>
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "hal/config.h"
#include "hal/inputThread.hpp"
 
 class rcInputClass : public inputThreadClass {
	 public:
		rcInputClass (std::string devpath);	/**< Create a rcInput reader attached to serial port devpath 		*/
		rcInputClass (void);			/**< Create a rcInput reader attached to serial port defined in hal/config.h */
		int getThrottle (void);			/**< Get the last throttle position from the RC input 				*/
		int getRudder (void);			/**< Get the last rudder position from the RC input 				*/
		rcModeEnum getMode (void);		/**< Get the last mode command from the RC input 					*/	
		
		~rcInputClass();			/**< Explicit destructor to make sure we close out the serial port and kill the thread.	*/
				
	private:
		int _throttle = 0;
		int _rudder = 0;
		rcModeEnum _mode = NONE;
 };
 
#endif
