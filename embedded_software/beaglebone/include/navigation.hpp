/******************************************************************************
 * Hackerboat Beaglebone navigation module
 * nav.hpp
 * This module stores locations and functions for manipulating them
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef NAVIGATION_H
#define NAVIGATION_H 
 
#include <jansson.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "config.h"
#include "stateStructTypes.hpp"
#include "location.hpp"

#include <string>
#include <vector>

/**
 * @class navVectorClass
 *
 * @brief Class for storing a navigation vector
 *
 */

class navVectorClass : public hackerboatStateClass {
	public:	
		navVectorClass (void) {};
		navVectorClass (std::string src, double bearing, double strength);
		bool isValid (void) const;				/**< Check for validity */	
		bool norm (void);						/**< Normalize the bearing */
		bool parse (json_t *input);		/**< Populate the object from the given json object */
		json_t *pack (void);			/**< Pack the contents of the object into a json object and return a pointer to that object*/
		navVectorClass add (navVectorClass a);	/**< Vector sum of the current vector and another vector */
		
		string	_source  	= "";		/**< Name of the source of this vector. */
		double 	_bearing 	= NAN;		/**< Bearing of this vector in degrees, clockwise from true north. */
		double	_strength 	= NAN;		/**< Relative strength of this vector */	

	private:
		static const string _format = "{s:s,s:f,s:f}";	
};

/**
 * @class navClass
 *
 * @brief Stores the current state of the navigation computation 
 *
 */

class navClass : public hackerboatStateClassStorable {
	public:
		navClass (void) {};	
		bool parse (json_t *input, bool seq = true);/**< Populate the object from the given json object. If seq is true, a sequence number element is expected */
		json_t *pack (bool seq = true);				/**< Pack the contents of the object into a json object and return a pointer to that object. If seq is true, a sequence number element will be included */
		bool appendVector (navVectorClass vec);	/**< Add a navigation vector to the influence list */
		bool calc (double maxStrength);			/**< Calculate the course to the next waypoint and sum the navInfluences vectors */
		void clearVectors (void);				/**< Clear the contents of navInfluences */
		bool isValid(void) const;
		
		locationClass	current;		/**< current location */	
		waypointClass	target;			/**< target waypoint */
		double			waypointStrength;
		double			magCorrection = 0;	/**< Correction between sensed magnetic heading and true direction */
		navVectorClass		targetVec;		/**< Vector to the target */
		navVectorClass		total;			/**< Sum of target vector and all influences */

	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
		virtual bool fillRow(sqliteParameterSlice) const;
		virtual bool readFromRow(sqliteRowReference, sequence);

	private:
		static const string _format = "{s:o,s:o,s:f,s:f,s:o,s:o,s:[o]}";	
		std::vector<navVectorClass>				navInfluences;	/**< Array to hold the influences of other navigation sources (i.e. collision avoidance) */
};

/** 
 * @class navigatorBase
 *
 * @brief Pure virtual base class for navigation elements other than waypoint
 *
 */

class navigatorBase {
	public:
		virtual navVectorClass calc(void) = 0;
		virtual bool isValid(void) const = 0;
	protected:
		navigatorBase(void) = default;
};

/**
 * @class navDodgePointClass
 *
 * @brief 
 *
 */

class navDodgePointClass : public navigatorBase {
	public:
		navDodgePointClass(void) {};
		navDodgePointClass(locationClass point) {_point = point;};
		navDodgePointClass(locationClass point, double strength)
                    : _point(point), _strength(strength)
		{ }
		navVectorClass calc(void);
		void setLocation (locationClass point) {_point = point;};
		locationClass getLocation (void) {return _point;};
		void setStrength (double strength) {_strength = strength;};
		double getStrength (void) {return _strength;};
		bool isValid(void) const;
	private:
		locationClass _point;
		double _strength = 0;
};
 
/** 
 * @class navDitherClass
 *
 * @brief
 *
 */

class navDitherClass : public navigatorBase {
	public:
		navDitherClass(void) {};
		navDitherClass(double strength) :_strength(strength) {};
		navDitherClass(double strength, uint32_t seed) :
			_seed(seed), _strength(strength) {};
		navVectorClass calc(void);
		void setStrength (double strength) {_strength = strength;};
		double getStrength (void) {return _strength;};
		void setSeed (double seed) {_seed = seed;};
		double getSeed (void) {return _seed;};
		bool isValid(void) const {return (isnormal(_strength) && isnormal(_seed));};
	private:
		double 		_strength = 0;
		uint32_t	_seed = HASHSEED;
};
 
#endif /* NAVIGATION_H */ 
