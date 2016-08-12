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

class inputThreadClass {
	public:
		inputThreadClass();
		virtual bool begin();
		virtual bool lock();
		virtual bool unlock();
		virtual bool unlockWait(int ms);
		virtual bool execute();				/**< Gather input	*/
		void runThread() {					/**< Thread runner function */
			runFlag = true;
			if (!(this->begin())) return;
			while (runFlag) {
				this->execute();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		void kill() {runFlag = false;}		/**< Kill the thread */
		std::chrono::time_point<std::chrono::system_clock> getLastInputTime() {
			return lastInput;
		}
		
	protected:
		std::chrono::time_point<std::chrono::system_clock> lastInput;	/**< Time that last input was processed */
		virtual bool forceUnlock(int ms);
		
	private:
		atomic_bool runFlag = false;
	
};


#endif 