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

int main (void) {
	gpsFixClass myFix;
	std::string input;
	uint32_t cnt = 0;
	char buf[1024];
	
	logError::instance()->open(NAV_LOGFILE);	// open up the error logfile
	
	// attempt to open the serial port
	while (1) {
		if (myFix.openGPSserial() != -1) break;	// if the serial port opened, leave. 
		logError::instance()->write("GNSS", "Failed to open serial port");
		usleep(500);
		cnt++;
	}
	
	for (;;) {
		
		memset (&buf, '\0', sizeof buf);
		ssize_t n = read(myFix.getFD(), buf, sizeof(buf));
		input.apped(buf, n);
		// find the start of an NMEA sentences
		size_t startPos = input.find_first_of("$", 0);
		if (startPos != std::string::npos) {
			// find the end of the sentence
			size_t endPos = input.find_first_of("\n\r", startPos);
			if (endPos != std::string::npos) {
				// get the NMEA sentence as a substring
				std::string sentence = input.substr(startPos, (endPos - startPos));
				// drop the sentence from the input buffer
				input = input.substr(endPos);
				if (minmea_check(sentence.c_str(), true)) {
					myFix.getLastRecord();					// get the last record -- if there's nothing in the DB, this will fail silently, leaving a blank record to populate
					if (myFix.readSentence(sentence)) {		// parse the sentence; if the result is valid...
						myFix.appendRecord();				// write it to the database
					}
				} else {
					logError::instance()->write("GNSS", "Received bad NMEA sentence");
					sentence.clear();
				}
			}
		} else input.clear();
		usleep(10000);
	}
	myFix.closeGPSserial()	
}
