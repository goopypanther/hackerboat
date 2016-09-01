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

#include <cstdlib>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include "hackerboatRoot.hpp"

/**
 * @brief This class defines a common interface and routines for creating input threads.
 */
 
class InputThread {
	public:
		InputThread() = default;	
		
		virtual bool begin() = 0;				/**< Start the input thread */
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
		sysclock getLastInputTime() {			/**< Get the time the last data arrived. */
			return lastInput;
		};
		
		std::unique_lock<std::timed_mutex> lock {mtx, std::defer_lock};	/**< Locking object for data */
	
	protected:
		void setLastInputTime() {lastInput = system_clock::now();};
		
	private:
		sysclock lastInput;						/**< Time that last input was processed */
		std::atomic_bool runFlag { false };
		std::timed_mutex mtx;
	
};


#endif 