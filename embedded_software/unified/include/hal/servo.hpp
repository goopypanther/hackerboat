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

#define MIN_PULSE_WIDTH			1000     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      	2000     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  	1500     // default pulse width when servo is attached
#define DEFAULT_FREQUENCY    	50     // minumim time to refresh servos in microseconds 

class Servo {
	public:
		Servo();
		bool attach(std::string pinName, 
					int min = MIN_PULSE_WIDTH,
					int max = MAX_PULSE_WIDTH,
					int freq = DEFAULT_FREQUENCY);		/**< Attach servo to named pin. Returns true if successful. */
		bool attach(int port, 
					int min = MIN_PULSE_WIDTH,
					int max = MAX_PULSE_WIDTH,
					int freq = DEFAULT_FREQUENCY);		/**< Attach servo to named pin. Returns true if successful. */
		void detach();
		void write(int value);
		void writeMicroseconds(unsigned int value);
		bool setFrequency(unsigned int freq);
		int read();
		unsigned int readMicroseconds();
		bool attached();
	private:
		unsigned int _min = 1000;
		unsigned int _max = 2000;
		unsigned int _freq = 100;
	
};

#endif 