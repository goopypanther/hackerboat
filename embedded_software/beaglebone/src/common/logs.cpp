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

bool logError::open(std::string path)
{
#warning Implement me
	return true;
}

bool logError::write(std::string source, std::string message)
{
	std::cerr << source << ": " << message << std::endl;
	return true;
}

logError logError::_instance;
