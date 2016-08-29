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
#include "hackerboatRoot.hpp"
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
	std::ostringstream out;
	out << "[" << HackerboatState::packTime(system_clock::now()) << "]";
	return out.str();
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
