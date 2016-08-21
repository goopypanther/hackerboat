/******************************************************************************
 * Hackerboat threaded input module
 * hal/inputThread.hpp
 * This module provides common routines for all threaded input readers
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H

#include <stdlib.h>
#include <atomic>
#include <thread>
#include <chrono>

/**
 * @brief This class defines a common interface and routines for creating input threads.
 */
 
using std::chrono;

class inputThreadClass {
	public:
		inputThreadClass() = default;					
		virtual bool begin();					/**< Start the input thread */
		virtual bool lock(duration dur);		/**< Lock the thread's data for the given duration (for example, to read data) */
		virtual bool unlock();					/**< Unlock the thread's data. */	
		virtual bool unlockWait(duration dur);	/**< Wait for the given duration to unlock the data. */
		virtual bool execute();					/**< Gather input	*/
		void runThread() {						/**< Thread runner function */
			runFlag = true;
			if (!(this->begin())) return;
			while (runFlag) {
				this->execute();
				std::this_thread::sleep_for(milliseconds(1));
			}
		}
		void kill() {runFlag = false;}			/**< Kill the thread */
		time_point<system_clock> getLastInputTime() {	/**< Get the time the last data arrived. */
			return lastInput;
		}
		
	protected:
		time_point<system_clock> lastInput;	/**< Time that last input was processed */
		virtual bool forceUnlock();			/**< Force the data to unlock. */
		
	private:
		atomic_bool lockFlag = false;
		atomic_bool runFlag = false;
	
};


#endif 