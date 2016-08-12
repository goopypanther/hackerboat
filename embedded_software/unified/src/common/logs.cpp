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
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

std::string timeOutput (void) {
	auto logTime = system_clock::now();
	time_t logTimeT = system_clock::to_time_t(logTime)
	char timebuf[LOCAL_BUF_LEN];
	
	strftime(timebuf, LOCAL_BUF_LEN, "[%F-%R:%S.", logTimeT);
	std::string	ret(timebuf);
	// this needs to be replaced with the appropriate thing to give me decimal seconds, but I haven't figured that out yet
	//ret += std::to_string(logTime.tv_nsec);
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
