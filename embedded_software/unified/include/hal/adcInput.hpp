/******************************************************************************
 * Hackerboat ADC input module
 * hal/adcInput.hpp
 * This module reads analog input data
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
#include "hal/inputThread.hpp"
#include "hal/config.h"
#include "hal/drivers/adc128d818.hpp"

class ADCInput : public InputThread {
	public:
		ADCInput(void) {inputsValid = init();};	
		
		bool 							isValid() {return inputsValid;};				/**< Check if the hardware connections are good */
		bool							init();											/**< Intialize all inputs */
		bool 							begin();										/**< Start the input thread */
		bool 							execute();										/**< Gather input	*/
		std::map<std::string, int> 		getRawValues (void) {return _raw;};				/**< Return the raw ADC values, in volts */
		std::map<std::string, double> 	getScaledValues (void);							/**< Return the scaled ADC values */
		bool 							setOffsets (std::map<std::string, int> offsets);/**< Set the offsets for all channels. */
		bool 							setScales (std::map<std::string, double> scales);/**< Set the scaling for all channels. */
		std::map<std::string, int> 		getOffsets() {return _offsets;};				/**< Get the offsets for all channels. */
		std::map<std::string, double> 	getScales() {return _scales;};					/**< Get the scaling for all channels. */
		~ADCInput () {
			this->kill(); 
		}
		
		using InputThread::getLastInputTime;
		
	private:
		ADC128D818 						upper { ADC_UPPER_ADDR, ADC_I2C_BUS };
		ADC128D818 						lower { ADC_LOWER_ADDR, ADC_I2C_BUS };
		std::vector<std::string>		upperChannels ADC_UPPER_INITIALIZER;
		std::vector<std::string>		lowerChannels ADC_LOWER_INITIALIZER;
		std::string						batmonPath;
		std::map<std::string, int> 		_raw;
		std::map<std::string, int> 		_offsets;
		std::map<std::string, double> 	_scales;
		bool							inputsValid = false;
		std::thread *myThread;
};

#endif /* ADCINPUT_H */
