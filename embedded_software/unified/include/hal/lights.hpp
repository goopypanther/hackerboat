/******************************************************************************
 * Hackerboat lights module
 * hal/lights.hpp
 * This module controls the lights using PixelBone
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef LIGHTS_H
#define LIGHTS_H


#include "enumdefs.hpp"
#include "pixel.hpp"

class HalTestHarness;

class Lights {
	friend class HalTestHarness;
	public:
		Lights() = default;
		void setBrightness(int bright);		/**< Set the brightness of the whole string */
		void setMode(BoatModeEnum boat, 	/**< Set the lights as appropiate for the given modes */
					NavModeEnum nav, 
					AutoModeEnum autonomy, 
					RCModeEnum rc);
		void clear();						/**< Turn off all the lights */
		
	private:
		PixelBone_Pixel 	strip { LIGHTS_COUNT };
		BoatModeEnum 		_boat; 
		NavModeEnum 	_nav; 
		AutoModeEnum 		_auto; 
		RCModeEnum 			_rc;
};

#endif
