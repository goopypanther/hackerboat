/******************************************************************************
 * Hackerboat dodge module
 * dodge.hpp
 * This module takes the incoming AIS data and produces a dodge vector
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef DODGE_H
#define DODGE_H

#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include "location.hpp"
#include "ais.hpp"
#include "gps.hpp"
#include "hackerboatRoot.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/config.h"
#include "twovector.hpp"

using namespace std;

/**
 * @brief The dodge class provides a mechanism for determining the diversion required by nearby vessels.
 */
class Dodge {
	public:
		Dodge (GPSdInput& input) : 
			_in(input) {};
		
		TwoVector calcDodge ();		/**< calculates the dodge vector and returns it*/
		TwoVector getDodge ();		/**< Returns the dodge vector. Values are bearing in degrees from magnetic north and relative strength */
		
		sysclock lastCalc;
		
	private:
		TwoVector singleDodge (const Location me, const Location them, const AISShipType theirType);
		TwoVector lastDodge;
		GPSdInput& 	_in;
		map<AISShipType, tuple<double, double>>		dodge;	/**< Strength of dodge and minimum distance in meters (respectively) for each ship type. */ 
};

#endif /* DODGE_H */