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
#include <atomic>
#include <thread>
#include <fstream>
#include <iostream>
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "hal/config.h"
#include "hal/inputThread.hpp"
 
class RCInput : public InputThread {
	public:
		RCInput (std::string devpath);	/**< Create a rcInput reader attached to serial port devpath 		*/
		RCInput (void);					/**< Create a rcInput reader attached to serial port defined in hal/config.h */
		int getThrottle (void);			/**< Get the last throttle position from the RC input 				*/
		double getRudder (void);		/**< Get the last rudder position from the RC input 				*/
		double getCourse ();			/**< Get the last course command, in degrees. */	
		int getChannel (int channel);	/**< Return the raw value of the given channel */
		bool isValid ();
		bool isFailSafe ();				/**< Returns true if in failsafe mode. */
			
		~RCInput();						/**< Explicit destructor to make sure we close out the serial port and kill the thread.	*/
				
	private:
		int _throttle = 0;
		double _rudder = 0;
		std::string _path;
		int devFD;
};
 
#endif
