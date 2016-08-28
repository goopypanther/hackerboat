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
#include <sstream>

using namespace std;
using namespace std::chrono;

std::string timeOutput (void) {
	auto logTime = system_clock::now();
	auto minTime = time_point_cast<minutes>(logTime);
	auto millis = duration_cast<milliseconds>(logTime - minTime);
	struct tm * myUTCtime;
	time_t myEpochTime;
	char timebuf[LOCAL_BUF_LEN];
	
	myEpochTime = system_clock::to_time_t(logTime);
	myUTCtime = gmtime(&myEpochTime);
	strftime(timebuf, LOCAL_BUF_LEN, "[%F-%R:", myUTCtime);
	std::stringstream ret(timebuf);
	ret << (millis.count()/1000) << "]:";
	free(myUTCtime);
	return ret.str();
}

bool LogError::open(std::string path)
{
	log.open(path);
	return log.is_open();
}

bool LogError::close(void)
{
	log.close();
	return !log.is_open();
}

bool LogError::write(std::string source, std::string message)
{
	log << timeOutput() << source << ": " << message << std::endl;
	return true;
}

LogError LogError::_instance;
