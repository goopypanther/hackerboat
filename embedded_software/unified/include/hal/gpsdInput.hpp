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
#include <map>
#include <list>
#include "hal/config.h"
#include "gps.hpp"
#include "ais.hpp"
#include "hal/inputThread.hpp"
#include "pstream.h"
#include "location.hpp"

class HalTestHarness;

using namespace std;

class GPSdInput : public InputThread {
	friend class HalTestHarness;
	public:
		GPSdInput();					
		GPSdInput(string host, int port);			/**< Create a gpsd object pointing at the given host & port combination. */
		bool setHost (string host);					/**< Point the input listener at the given host. */
		bool setPort (int port);					/**< Point the input listener at the given port */
		bool connect ();							/**< Connect to the host */
		bool disconnect ();							/**< Disconnect from the host. */
		bool isConnected ();						/**< Returns true if connected. */
		bool begin();								/**< Start the input thread */
		bool execute();								/**< Gather input	*/
		GPSFix* getFix() {return &_lastFix;};		/**< Returns last GPS fix (TSV report, more or less) */
		std::map<int, AISShip> getData();				/**< Returns all AIS contacts */
		std::map<int, AISShip> getData(AISShipType type);/**< Returns AIS contacts of a particular ship type */
		AISShip getData(int MMSI);					/**< Returns AIS contact for given MMSI, if it exists. It returns a reference to a default (invalid) object if the given MMSI is not present. */
		AISShip getData(string name);				/**< Returns AIS contact for given ship name, if it exists. It returns a reference to a default (invalid) object if the given ship name is not present. */
		int pruneAIS(Location loc);					/**< Call the prune() function of each AIS contact. */
		bool isValid() {return isConnected();};
		GPSFix getAverageFix();
		~GPSdInput () {
			this->kill(); 
			//if (myThread) delete myThread;
		}
		
	private:
		string 				_host = "127.0.0.1";
		int 				_port = 3001;
		GPSFix 				_lastFix;
		GPSFix				_averageFix;
		list<GPSFix>		_gpsAvgList;
		std::map<int, AISShip>	_aisTargets;
		redi::pstreambuf	gpsdstream;
		std::thread 		*myThread;
};
		
#endif
