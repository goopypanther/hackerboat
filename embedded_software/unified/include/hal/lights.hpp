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

#include "config.h"
#include "enumdefs.h"
#include "pixel.hpp"

class lightsClass {
	public:
		lightsClass();
		void setBrightness(int bright);
		void setMode(boatModeEnum boat, navigationModeEnum nav, autoModeEnum autonomy, rcModeEnum rc);
		void clear();
		
	private:
		Pixelbone_Pixel strip(LIGHTS_COUNT);
		boatModeEnum _boat; 
		navigationModeEnum _nav; 
		autoModeEnum _auto; 
		rcModeEnum _rc;
};

#endif