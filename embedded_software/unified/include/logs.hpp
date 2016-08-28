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
		static LogError* instance(void) { return &_instance; }
		bool open(std::string logfile);
		bool write(const std::string source, const std::string message);
		bool close(void);
	private:
		LogError(void) = default;
		LogError(LogError const&) = delete;
		LogError& operator=(LogError const&) = delete;
		static LogError _instance;
		ofstream log;
};

#endif /* RESTDISPATCH_H */
