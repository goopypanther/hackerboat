/******************************************************************************
 * Hackerboat Beaglebone Servo module
 * hal/servo.hpp
 * This module manipulates the servo output
 * see the Hackerboat documentation for more details
 * Code is derived, in a general sense, from the Arduino Servo library
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef SERVO_H
#define SERVO_H
 
#include "config.h"
#include <math.h>
#include <string>
#include <inttypes.h>

#define MIN_PULSE_WIDTH			1000	// the shortest pulse sent to a servo, microseconds  
#define MAX_PULSE_WIDTH      	2000	// the longest pulse sent to a servo, microseconds 
#define DEFAULT_PULSE_WIDTH  	1500	// default pulse width when servo is attached
#define DEFAULT_FREQUENCY    	100		// servo frequency, Hz

class Servo {
	public:
		Servo();
		bool attach(int port, 							/**< Attach servo to named pin on the named port {8|9}. Returns true if successful. */
					int pin,
					long min = MIN_PULSE_WIDTH,
					long max = MAX_PULSE_WIDTH,
					long freq = DEFAULT_FREQUENCY);		
		void detach();									/**< Detach servo from the pin */
		bool write(double value);						/**< Set the servo position, from -100.0 to +100.0. Returns true if successful, false is unsuccessful or out of range. */
		bool writeMicroseconds(unsigned long value);	/**< Set the servo position in microseconds. Returns true if successful, false is unsuccessful or out of range. */
		bool setFrequency(unsigned long freq);			/**< Set servo frequency in Hz. Returns true if successful. */
		bool setMax (unsigned long max);				/**< Set the maximum servo value, in microseconds. Returns false if max time period is longer than one time period at the current frequency */
		bool setMin (unsigned long min);				/**< Set the minimum servo value, in microseconds. Returns false is larger than max */
		long getMax () {return _max/1000.0;};
		long getMin () {return _min/1000.0;};
		double read();									/**< Read the current servo value. */
		unsigned long readMicroseconds();				/**< Read the current servo value, in microseconds. */
		bool isAttached() {return attached;};			/**< Check if this object is attached to a pin. */	
	private:
		std::string getServoPath (int port, int pin);
		bool writeMicroseconds();						/**< Set pwm channel to the currently stored frequency */
		bool setFrequency();							/**< Set pwm channel to the currently stored frequency */

		std::string path = "";
		std::string pinname = "";
		int majornum = -1;
		int minornum = -1;
		// all time values in nanoseconds
		unsigned long _min = 1000000;
		unsigned long _max = 2000000;
		unsigned long _freq = 20000000;
		unsigned long _center = 1500000;
		unsigned long _val = 1500000;
		bool attached = false;
	
};

#endif 