/******************************************************************************
 * Hackerboat sim module
 * HackerboatHALsim.hpp
 * This module pretends to be the HAL in order to provide a simulated environment
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Apr 2017
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
extern 'C' {
	#include <jansson.h>
}
#include <cstdlib>
#include <chrono>
#include <string>
#include <thread>
#include "hackerboatRoot.hpp"
#include "location.hpp"
#include "orientation.hpp"
#include "gps.hpp"
#include "hal/config.h"
#include "hal/servo.hpp"
#include "hal/throttle.hpp"
#include "hal/gpsdInput.hpp"
#include "hal/inputThread.hpp"

using namespace std;

class HackerboatHALsim {
	friend class GPSdInput;
	friend class OrientationInput;
	friend class Servo;
	friend class Throttle;
		
	protected:
		bool execute();
		bool setRudder (double rudder);
		bool setThrottle (double throttle);
		HackerboatHALsim *getsim () {return _instance;};
		
		Location currentLocation;
		GPSFix currentFix;
		Orientation currentOrientation; 
		bool runFlag = false;
		
		class SimThreadRunner {
			public:
				SimThreadRunner (HackerboatHALsim *mine) : me(mine) {};
				void operator()() {						/**< Thread runner function */
					me->runFlag = true;
					while (me->runFlag) {
						auto endtime = std::chrono::system_clock::now() + me->period;
						me->execute();
						std::this_thread::sleep_until(endtime);
					}
				};
				
			private:
				HackerboatHALsim *me;	// The simulation this function is running
		};
		friend SimThreadRunner
		
		void kill() {runFlag = false;};			/**< Kill the thread */
		
	private:
		HackerboatHALsim ();												/**< Hark, a singleton! */
		HackerboatHALsim (HackerboatHALsim const&) = delete;				/**< Hark, a singleton! */
		HackerboatHALsim& operator=(HackerboatHALsim const&) = delete;		/**< Hark, a singleton! */
	
		static HackerboatHALsim* _instance;									/**< Hark, a singleton! */
		
		bool internalsim = true;	/**< True if running internal simulation, otherwise false */
		bool waiting = false;		/**< True if we have one of throttle or rudder setting and we're waiting for the other one; otherwise false */
	
		bool load ();													/**< Initialize the sim from the default config.json file */
		bool load (const string &path);									/**< Initialize the sim from the given config.json file */

		// thread stuff
		std::thread 		*myThread;
		
		// inputs and outputs for external sim mode
		istream *input;				/**< Stream to read new position and heading data from */
		ostream *output;			/**< Stream to right throttle and rudder data to */
		
		// configuration for internal sim mode
		ifstream* confFile = "";				/**< Simulation configuration file path (necessary for both internal and external sim */
		Location startpoint;					/**< Starting point, derived from configuration file */
		vector<double> rudderpolynomial;		/**< Polynomial for turning rudder position into a rate of turn */
		vector<double> throttlepolynomial;		/**< Polynomial for turning throttle position into a speed in m/s */
		double compassError = 0;				/**< Error in the compass reading */
		
		// sim timing
		std::chrono::system_clock::duration 	period = 1ms;	// This is really just waiting for input and output from other threads -- the only reason for this to be here is to keep the thread from lagging the system
		std::chrono::system_clock::time_point	lastSimStep;
};

