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
		bool attach(std::string pinName, 				/**< Attach servo to named pin. Returns true if successful. */
					int min = MIN_PULSE_WIDTH,
					int max = MAX_PULSE_WIDTH,
					int freq = DEFAULT_FREQUENCY);		
		bool attach(int port, 							/**< Attach servo to named pin on the named port {8|9}. Returns true if successful. */
					int pin,
					int min = MIN_PULSE_WIDTH,
					int max = MAX_PULSE_WIDTH,
					int freq = DEFAULT_FREQUENCY);		
		void detach();									/**< Detach servo from the pin */
		bool write(double value);						/**< Set the servo position, from -100.0 to +100.0. Returns true if successful, false is unsuccessful or out of range. */
		bool writeMicroseconds(unsigned int value);		/**< Set the servo position in microseconds. Returns true if successful, false is unsuccessful or out of range. */
		bool setFrequency(unsigned int freq);			/**< Set servo frequency. Returns true if successful. */
		double read();									/**< Read the current servo value. */
		unsigned int readMicroseconds();				/**< Read the current servo value, in microseconds. */
		bool isAttached();								/**< Check if this object is attached to a pin. */	
	private:
		unsigned int _min = 1000;
		unsigned int _max = 2000;
		unsigned int _freq = 100;
	
};

#endif 