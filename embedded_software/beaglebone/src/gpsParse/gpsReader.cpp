/******************************************************************************
 * Hackerboat Beaglebone GPS parser program
 * gpsReader.cpp
 * This program reads in GPS strings
 * see the Hackerboat documentation for more details
 * 
 * Written by Pierce Nichols, Apr 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>
#include <limits>
#include "stateStructTypes.hpp"
#include "config.h"
#include "logs.hpp"
#include "navigation.hpp"
#include "location.hpp"
#include "boneState.hpp"
#include "sqliteStorage.hpp"
#include <minmea.h>
#include <BlackUART/BlackUART.h>

using namespace BlackLib;

int main (void) {
	gpsFixClass myFix;
	BlackUART port(GNSS_UART, GNSS_BAUD, ParityNo, StopOne, Char8);
	std::string input;
	
	logError::instance()->open(NAV_LOGFILE);	// open up the logfile
	
	// attempt to open the serial port
	while (!port.open(ReadOnly)) {
		logError::instance()->write("GNSS", "Failed to open serial port");
		usleep(500);
	}
	
	for (;;) {
		input += port.read();
		// find the start of an NMEA sentences
		size_t startPos = input.find_first_of("$", 0);
		if (startPos != std::string::npos) {
			// find the end of the sentence
			size_t endPos = input.find_first_of("\n\r", 0);
			if (endPos != std::string::npos) {
				// get the NMEA sentence as a substring
				std::string sentence = input.substr(startPos, (endPos - startPos));
				// drop the sentence from the input buffer
				input = input.substr(endPos);
				if (minmea_check(sentence.c_str(), true)) {
					if (myFix.getLastRecord() && myFix.readSentence(sentence)) {
						myFix.appendRecord();
					}
				} else {
					logError::instance()->write("GNSS", "Received bad NMEA sentence");
					sentence.clear();
				}
			}
		} else input.clear();
		usleep(10000);
	}
	port.close();	
}
