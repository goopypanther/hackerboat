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
#include "hal/drivers/adc128d818.hpp"
#include "configuration.hpp"

class HalTestHarness;

using namespace std;

class ADCInput : public InputThread {
	friend class HalTestHarness;
	public:
		ADCInput(void); 
		
		bool 				isValid() {return inputsValid;};				/**< Check if the hardware connections are good */
		bool				init();											/**< Intialize all inputs */
		bool 				begin();										/**< Start the input thread */
		bool 				execute();										/**< Gather input	*/
		map<string, int> 	getRawValues (void) {return _raw;};				/**< Return the raw ADC values, in volts */
		map<string, double> getScaledValues (void);							/**< Return the scaled ADC values */
		bool 				setOffsets (std::map<std::string, int> offsets);/**< Set the offsets for all channels. */
		bool 				setScales (std::map<std::string, double> scales);/**< Set the scaling for all channels. */
		map<string, int> 	getOffsets() {return _offsets;};				/**< Get the offsets for all channels. */
		map<string, double> getScales() {return _scales;};					/**< Get the scaling for all channels. */
		~ADCInput () {
			this->kill(); 
			//if (myThread) delete myThread;
		}
		
		using InputThread::getLastInputTime;
		
	private:
		ADC128D818 			upper { Conf::get()->adcUpperAddress(), Conf::get()->adcI2Cbus() };
		ADC128D818 			lower { Conf::get()->adcLowerAddress(), Conf::get()->adcI2Cbus() };
		vector<string>		upperChannels = Conf::get()->adcUpperChanList();
		vector<string>		lowerChannels = Conf::get()->adcLowerChanList();
		string				batmonPath;
		map<string, int> 	_raw;
		map<string, int> 	_offsets;
		map<string, double> _scales;
		bool				inputsValid = false;
		thread *myThread = NULL;
};

#endif /* ADCINPUT_H */
