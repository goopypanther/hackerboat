/******************************************************************************
 * Hackerboat ADC input module
 * hal/adcInput.hpp
 * This module reads orientation data
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ADCINPUT_H
#define ADCINPUT_H

#include <string>
#include <stdlib.h>
#include <time.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include "hal/config.h"
#include "hal/adc128d818.hpp"

class adcInputClass : public inputThreadClass {
	public:
		adcInputClass(void);	
		map<std::string, int> getRawValues (void);			/**< Return the ADC values */
		map<std::string, double> getScaledValues (void);	/**< Return the ADC values */
		
	private:
		adc128d818	upper(ADC_UPPER_ADDR, ADC_I2C_BUS);
		adc128d818	lower(ADC_LOWER_ADDR, ADC_I2C_BUS);
		map<std::string, int> _raw;
		map<std::string, int> _offsets;
		map<std::string, double> _scales;
};

#endif