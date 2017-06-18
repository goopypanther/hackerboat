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

#include "hal/adcInput.hpp"

using namespace std;

class HealthMonitor : public HackerboatState {
	public:
		HealthMonitor () = default;
		HealthMonitor (ADCInput* adc) : _adc(adc) {};
		bool parse (Value& input);
		Value pack () const;
		bool isValid () {return valid;};
		bool setADCdevice(ADCInput* adc) {	/**< Set the ADC input thread */
			valid = false;
			if (adc) {
				_adc = adc;
				return true;
			}
			return false;
		}
		bool readHealth ();					/**< Update health monitor values */
		
		double		servoCurrent;			/**< Current supplied to the servo, amps */
		double		batteryMon;				/**< Battery voltage, volts */
		double		mainVoltage;			/**< Main bus voltage, volts */
		double		mainCurrent;			/**< Main bus current, amps */
		double		chargeVoltage;			/**< Charging voltage, volts */
		double		chargeCurrent;			/**< Charging current, amps */
		double		motorVoltage;			/**< Motor voltage, volts */
		double		motorCurrent;			/**< Motor current, amps */
		int			rcRssi;					/**< RC system RSSI, dbm */
		int			cellRssi;				/**< Cell system RSSI, dbm */
		int			wifiRssi;				/**< Wifi RSSI, dbm */
		
	private:
		bool valid;
		ADCInput* _adc;
};


#endif /* HEALTHMON_H */