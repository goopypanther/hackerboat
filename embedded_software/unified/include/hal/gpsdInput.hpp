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
#include <vector>
#include <tuple>
#include "hal/config.h"
#include "gps.hpp"
#include "ais.hpp"
#include "hal/inputThread.hpp"

using std;

class gpsdInputClass : public inputThreadClass {
	public:
		gpsdInputClass();
		gpsdInputClass(string host, int port);
		gpsFixClass getFix();
		vector<tuple<int, aisBaseClass>> getData();
		vector<tuple<int, aisBaseClass>> getData(aisShipType type);
		aisBaseClass getData(int MMSI);
		aisBaseClass getData(string name);
		int pruneAIS();
		
	private:
		string 								_host;
		int 								_port;
		gpsFixClass 						_lastFix;
		vector<tuple<int, aisBaseClass>>	_aisTargets;
		
#endif