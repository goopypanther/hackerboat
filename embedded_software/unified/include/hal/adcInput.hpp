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
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <inttypes.h>
#include "inputThread.hpp"
#include "hal/config.h"
#include "hal/drivers/adc128d818.hpp"

class ADCInput : public InputThread {
	public:
		ADCInput(void);	
		
		std::map<std::string, int> 		getRawValues (void);							/**< Return the raw ADC values */
		std::map<std::string, double> 	getScaledValues (void);							/**< Return the scaled ADC values */
		bool 							setOffsets (std::map<std::string, int> offsets);		/**< Set the offsets for all channels. */
		bool 							setScales (std::map<std::string, double> scales);	/**< Set the scaling for all channels. */
		std::map<std::string, int> 		getOffsets();									/**< Get the offsets for all channels. */
		std::map<std::string, double> 	getScales();									/**< Get the scaling for all channels. */
		
		using InputThread::getLastInputTime;
		
	private:
		ADC128D818 						upper { ADC_UPPER_ADDR, ADC_I2C_BUS };
		ADC128D818 						lower { ADC_LOWER_ADDR, ADC_I2C_BUS };
		std::string						batmonPath;
		std::map<std::string, int> 		_raw;
		std::map<std::string, int> 		_offsets;
		std::map<std::string, double> 	_scales;
};

#endif /* ADCINPUT_H */
