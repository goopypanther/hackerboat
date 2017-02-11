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
			if (raw) *raw = &(adc->_raw);
			if (valid) *valid = &(adc->inputsValid);
		}
		
		void accessGPSd (GPSdInput *gps, GPSFix **fix, std::map<int, AISShip> **targets) {
			if (fix) *fix = &(gps->_lastFix);
			if (targets) *targets = &(gps->_aisTargets);
		}
		
		void accessOrientation (OrientationInput *orient, Orientation **current, bool **valid) {
			if (current) *current = &(orient->_current);
			if (valid) *valid = &(orient->sensorsValid);
		}
		
		void accessRC (RCInput *rc, int **throttle, double **rudder, bool **failsafe, bool **valid,
						std::vector<uint16_t> **channels, std::string **buf, int **errs, int **good) {
							if (throttle) *throttle = &(rc->_throttle);
							if (rudder) *rudder = &(rc->_rudder);
							if (failsafe) *failsafe = &(rc->failsafe);
							if (valid) *valid = &(rc->_valid);
							if (channels) *channels = &(rc->rawChannels);
							if (buf) *buf = &(rc->inbuf);
							if (errs) *errs = &(rc->_errorFrames);
							if (good) *good = &(rc->_goodFrames);
						}
						
		void accessRelay (Relay *me, Pin **drive, Pin **fault) {
			if (drive) *drive = me->_drive;
			if (fault) *fault = me->_fault;
		}
};

#endif
 