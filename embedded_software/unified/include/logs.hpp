/******************************************************************************
 * Hackerboat Beaglebone logging module
 * logs.hpp
 * This modules is compiled into the other modules to give a common interface
 * for logging
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Dec 2015
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef LOGS_H
#define LOGS_H 
 
#include "hal/config.h" 

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class LogError {
	public:
		static LogError* instance(void) { return &_instance; }				/**< Get a pointer to the log instance */
		bool open(std::string logfile);										/**< Open the given log file */
		bool write(const std::string source, const std::string message);	/**< Write the given message about the given error source */
		bool close(void);													/**< Close the log file */
	private:
		LogError(void) = default;
		LogError(LogError const&) = delete;
		LogError& operator=(LogError const&) = delete;
		static LogError _instance;											/**< Singleton is singleton */
		ofstream log;														/**< Logfile output stream */
};

#endif /* RESTDISPATCH_H */
