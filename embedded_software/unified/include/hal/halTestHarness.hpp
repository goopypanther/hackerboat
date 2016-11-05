/******************************************************************************
 * Hackerboat HAL Test Harness module
 * hal/halTestHarness.hpp
 * This module permits manipulation of the I/O objects for writing useful unit tests
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Oct 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef HALTESTHARNESS_H
#define HALTESTHARNESS_H

#include "hal/adcInput.hpp"
#include "hal/gpio.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/orientationInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/throttle.hpp"
#include "hal/relay.hpp"
#include "hal/servo.hpp"
#include <string>
#include <iostream>

using namespace std;

class HalTestHarness {
	public:
		HalTestHarness () = default;
		void accessADC (ADCInput *adc, std::map<std::string, int> **raw, bool **valid) {
			*raw = &(adc->_raw);
			*valid = &(adc->inputsValid);
		}
		
		void accessGPSd (GPSdInput *gps, GPSFix **fix, std::map<int, AISShip> **targets) {
			*fix = &(gps->_lastFix);
			*target = &(gps->_aisTargets);
		}
		
		void accessOrientation (OrientationInput *orient, Orientation **current, bool **valid) {
			*current = &(orient->_current);
			*valid = &(orient->sensorsValid);
		}
		
		void accessRC (RCInput *rc, int **throttle, double **rudder, bool **failsafe, bool **valid,
						std::vector<uint16_t> **channels, std::string **buf, int **errs, int **good) {
							*throttle = &(rc->_throttle);
							*rudder = &(rc->_rudder);
							*failsafe = &(rc->failsafe);
							*valid = &(rc->_valid);
							*channels = &(rc->rawChannels);
							*buf = &(rc->inbuf);
							*errs = &(rc->_errorFrames);
							*good = &(rc->_goodFrames);
						}
						
		void accessRelay (Relay *me, Pin **drive, Pin **fault) {
			*drive = me->_drive;
			*fault = me->_fault;
		}
		
		void initGPIOTest (Pin *me, string path, string pin) {
			
		}
		
		void writeGPIOTest (Pin *me, int state) {
			
		}
		
		int readGPIOTest (Pin *me) {
			
		}
}

#endif
 