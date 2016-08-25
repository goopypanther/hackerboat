/******************************************************************************
 * Hackerboat Beaglebone AIS module
 * ais.cpp
 * This module stores AIS data 
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include "hal/config.h"
#include <math.h>
#include <string>
#include <chrono>
#include "hackerboatRoot.hpp"
#include "location.hpp
#include "hal/config.h"
#include "sqliteStorage.hpp"
#include "json_utilities.hpp"
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "ais.hpp"

AISShip::AISShip (json_t *packet) {
	recordTime = std::chrono::system_clock::now();
	parseGpsdPacket(packet);
}			

bool AISShip::parseGpsdPacket (json_t *input);
bool AISShip::parse (json_t *input);
bool AISShip::coreParse (json_t *input);

bool AISShip::project () {
	return project(std::chrono::system_clock::now());
}	
					
bool AISShip::project (sysclock);	
			
bool AISShip::prune (Location& current) {
	if ((!this->isValid()) || 
		(current.isValid() && (fix.distance(current) > AIS_MAX_DISTANCE)) ||
		((std::chrono::system_clock::now() - lastContact) > AIS_MAX_TIME)) {
		removeEntry();
		return true;
	} else return false;
}

json_t *AISShip::pack () const;

bool AISShip::isValid () const;

HackerboatStateStorage& AISShip::storage();
bool AISShip::fillRow(SQLiteParameterSlice) const USE_RESULT;
bool AISShip::readFromRow(SQLiteRowReference, sequence) USE_RESULT;