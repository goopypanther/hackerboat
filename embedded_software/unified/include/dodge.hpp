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
#include "logs.hpp"
#include "location.hpp"
#include "ais.hpp"
#include "gps.hpp"
#include "hackerboatRoot.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/config.h"

using namespace std;

/**
 * @brief The bearingVector class provides operations for working with vectors expressed as a bearing and a strength.
 */
class bearingVector : public hackerboatStateClass {
	public:
		bearingVector () {};
		bearingVector (double bearing, double strength) :
			_b(bearing), _s(strength) {};
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid ();
		
		static bearingVector sumVectors (bearingVector a, bearingVector b);
		void bearing (double b) {_b = b;};
		double bearing () {return _b;};
		void strength (double s) {_s = s;};
		double strength () {return _s;};
		
	private:
		double _b;
		double _s;
	
};

/**
 * @brief The dodge class provides a mechanism for determining the diversion required by nearby vessels.
 */
class dodgeClass {
	public:
		dodgeClass (gpsdInputClass* input) : _in(input) {};
		
		bearingVector calcDodge ();		/**< calculates the dodge vector and returns it*/
		bearingVector getDodge ();		/**< Returns the dodge vector. Values are bearing in degrees from magnetic north and relative strength */
		
		sysclock lastCalc;
		
	private:
		bearingVector singleDodge (const locationClass me, const locationClass them, const aisShipType theirType);
		bearingVector lastDodge;
		gpsdInputClass* 							_in;
		map<aisShipType, tuple<double, double>>		dodge;	/**< Strength of dodge and minimum distance in meters (respectively) for each ship type. */ 
};

#endif /* DODGE_H */