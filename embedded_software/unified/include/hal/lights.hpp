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

#include "hal/config.h"
#include "enumdefs.hpp"
#include "pixel.hpp"

class lightsClass {
	public:
		lightsClass() = default;
		void setBrightness(int bright);		/**< Set the brightness of the whole string *//
		void setMode(boatModeEnum boat, 	/**< Set the lights as appropiate for the given modes */
					navigationModeEnum nav, 
					autoModeEnum autonomy, 
					rcModeEnum rc);
		void clear();						/**< Turn off all the lights */
		
	private:
		Pixelbone_Pixel 	strip(LIGHTS_COUNT);
		boatModeEnum 		_boat; 
		navigationModeEnum 	_nav; 
		autoModeEnum 		_auto; 
		rcModeEnum 			_rc;
};

#endif
