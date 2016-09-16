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
#include <vector>
#include <inttypes.h>
#include "enumdefs.hpp"
#include "enumtable.hpp"
#include "hal/config.h"
#include "hal/inputThread.hpp"

#define SBUS_STARTBYTE			0x0f
#define SBUS_ENDBYTE			0x00
#define SBUS_BUF_LEN			25 
 
class RCInput : public InputThread {
	public:
		RCInput (std::string devpath = RC_SERIAL_PATH);	/**< Create a rcInput reader attached to serial port devpath 		*/
		int getThrottle ();				/**< Get the last throttle position from the RC input 				*/
		double getRudder ();			/**< Get the last rudder position from the RC input 				*/
		double getCourse ();			/**< Get the last course command, in degrees. */	
		RCModeEnum getMode();			/**< Returns the correct RC mode, given the current state of the inputs */
		int getChannel (int channel);	/**< Return the raw value of the given channel */
		bool isValid () {return _valid;};
		bool isFailSafe () {return failsafe;};	/**< Returns true if in failsafe mode. */
		bool begin();
		bool execute();
			
		~RCInput();						/**< Explicit destructor to make sure we close out the serial port and kill the thread.	*/
				
	private:
		int _throttle = 0;
		double _rudder = 0;
		std::string _path;
		int devFD = -1;
		bool failsafe = false;
		bool _valid = false;
		std::vector<uint16_t> rawChannels { RC_CHANNEL_COUNT };
		std::string inbuf;
		int _errorFrames = 0;
		int _goodFrames = 0;
		std::thread *myThread;
};
#endif
