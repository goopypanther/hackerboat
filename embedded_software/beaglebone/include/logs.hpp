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
 
#include "config.h" 

#include <string>

class logREST {
	public:
		static logREST* instance(void);
		bool open(std::string logfile);
		bool write(char** tokens, int tokenCount, char *query, char *body, int bodyLen, char *method, char *response);
		bool close(void);
	private:
		logREST(void){};
		logREST(logREST const&){};
		logREST& operator=(logREST const&){};
		static logREST* _instance;
};

class logError {
	public:
		static logError* instance(void);
		bool open(std::string logfile);
		bool write(const char *source, char *message);
		bool close(void);
	private:
		logError(void){};
		logError(logError const&){};
		logError& operator=(logError const&){};
		static logError* _instance;
};

#endif /* RESTDISPATCH_H */
