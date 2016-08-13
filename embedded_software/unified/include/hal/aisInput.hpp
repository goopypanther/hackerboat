/******************************************************************************
 * Hackerboat AIS input module
 * hal/aisInput.hpp
 * This module reads AIS data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef AISINPUT_H
#define AISINPUT_H

#include <string>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "ais.hpp"
#include "hal/config.h"
#include "hal/inputThread.hpp"
 
class aisInputClass : public inputThreadClass {
	public:
		aisInputClass (std::string host, int port);	/**< Create an aisInput reader attached to serial port devpath 		*/
		aisInputClass (void);					/**< Create a aisInput reader attached to serial port defined in hal/config.h */
		
		vector<aisClass> getData();
		vector<aisClass> getData(int type);
		aisClass getData(int MMSI);
		aisClass getData(std::string name);
		
	private:
		std::string _host;
		int _port;
		vector<aisClass> data;
}
#endif