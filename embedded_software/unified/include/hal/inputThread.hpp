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
#include "hackerboatRoot.hpp"

/**
 * @brief This class defines a common interface and routines for creating input threads.
 */
 
class InputThread {
	public:
		InputThread() = default;	
		
		virtual bool begin() = 0;				/**< Start the input thread */
		virtual bool lock(sysdur dur);		/**< Lock the thread's data for the given duration (for example, to read data) */
		virtual bool unlock();					/**< Unlock the thread's data. */	
		virtual bool unlockWait(sysdur dur);	/**< Wait for the given duration to unlock the data. */
		virtual bool execute() = 0;				/**< Gather input	*/
		void runThread() {						/**< Thread runner function */
			runFlag = true;
			if (!(this->begin())) return;
			while (runFlag) {
				this->execute();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		};
		void kill() {runFlag = false;};			/**< Kill the thread */
		sysclock getLastInputTime() {	/**< Get the time the last data arrived. */
			return lastInput;
		};
		
	protected:
		sysclock lastInput;	/**< Time that last input was processed */
		virtual bool forceUnlock();			/**< Force the data to unlock. */
		std::atomic_bool lockFlag { false };
		
	private:
		std::atomic_bool runFlag { false };
	
};


#endif 