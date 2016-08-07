/******************************************************************************
 * Hackerboat Beaglebone logging module
 * logs.hpp
 *
 * for logging
 * see the Hackerboat documentation for more details
 * 
 * Version 0.0: Nonfunctional stub implementation
 *
 ******************************************************************************/

#include "logs.hpp"
#include <iostream>
#include <fstream>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

std::string timeOutput (void) {
	timespec 	logTime;
	struct tm 	*thisTime;
	char 		timebuf[LOCAL_BUF_LEN];
	
	clock_gettime(CLOCK_REALTIME, &logTime);
	thisTime = localtime (&(logTime.tv_sec));
	strftime(timebuf, LOCAL_BUF_LEN, "[%F-%R:%S.", thisTime);
	std::string	ret(timebuf);
	ret += std::to_string(logTime.tv_nsec);
	ret += "]:";
	return ret;
}

bool logError::open(std::string path)
{
	log.open(path);
	return log.is_open();
}

bool logError::close(void)
{
	log.close();
	return !log.is_open();
}

bool logError::write(std::string source, std::string message)
{
	log << timeOutput() << source << ": " << message << std::endl;
	return true;
}

logError logError::_instance;

bool logREST::open(std::string path)
{
	log.open(path);
	return log.is_open();
}

bool logREST::close(void)
{
	log.close();
	return !log.is_open();
}

bool logREST::write(std::vector<std::string> tokens, std::string query, std::string method, std::string body, char *response) {
	log << "==============================================================" << std::endl;
	log << timeOutput() << std::endl;
	log << "URI: ";
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		log << "/" << *it;
	}
	log << std::endl << "Query: " << query << std::endl;
	log << "Method: " << method << std::endl;
	log << "Body: " << body << std::endl;
	log << "Response: " << std::string(response) << endl;
	
	return true;
}

logREST logREST::_instance;
