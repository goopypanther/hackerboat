/******************************************************************************
 * Hackerboat Beaglebone ADC Functional Test program
 * adc_test.cpp
 * This program is a functional test of the adc input subsystem
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include "hal/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <iostream>
#include <map>
#include "hal/adcInput.hpp"

using namespace std;

int main () {
	ADCInput adc;
	if (adc.begin() && adc.isValid()) {
		while (1) {
			std::map<std::string, int> raw = adc.getRawValues();
			std::map<std::string, double> scaled = adc.getScaledValues();
			for (auto const &r : raw) cout << r.first << "\t";
			cout << endl;
			for (auto const &r : raw) cout << r.second << "\t";
			cout << endl;
			for (auto const &r : scaled) cout << r.second << "\t";
			cout << endl;
		}
	}
}