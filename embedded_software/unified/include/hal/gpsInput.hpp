/******************************************************************************
 * Hackerboat orientation input module
 * hal/gpsInput.hpp
 * This module reads gps data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef GPSINPUT_H
#define GPSINPUT_H

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "hal/config.h"
#include "gps.hpp"
#include "hal/inputThread.hpp"

class gpsInputClass : public inputThreadClass {
	public:
		gpsInputClass();
		gpsInputClass(std::string host, int port);
		bool readSentence (std::string sentence);	/**< Populate class from incoming sentence string */
		gpsFixClass getFix();
		
	private:
		std::string _host;
		int 		_port;
		gpsFixClass _lastFix;
		

#endif