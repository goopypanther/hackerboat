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
