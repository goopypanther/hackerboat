/******************************************************************************
 * Hackerboat health monitor module
 * healthMonitor.hpp
 * This module monitors the health of the boat
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef HEALTHMON_H
#define HEALTHMON_H

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include "hackerboatRoot.hpp"
#include "hal/config.h"
#include "hal/adcInput.hpp"
#include "logs.hpp"
#include "hal/relay.hpp"

using namespace std;

class HealthMonitor : public HackerboatStateStorable {
	public:
		HealthMonitor () = default;
		HealthMonitor (ADCInput& adc) :
			_adc(adc) {};
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid () {return valid;};
		HackerboatStateStorage& storage();
		bool setADCdevice(ADCInput& adc);		/**< Set the ADC input thread */
		bool readHealth ();
		
		map<string, RelayTuple>	relays;			/**< State of the relays. Each tuple is current, state, and fault state */
		double					servoCurrent;	/**< Current supplied to the servo */
		double					batteryMon;
		double					mainVoltage;
		double					mainCurrent;
		double					chargeVoltage;
		double					chargeCurrent;
		double					motorVoltage;
		double					motorCurrent;
		int						rcRssi;
		int						cellRssi;
		int 					wifiRssi;
	private:
		bool valid;
		ADCInput& _adc;
};


#endif /* HEALTHMON_H */