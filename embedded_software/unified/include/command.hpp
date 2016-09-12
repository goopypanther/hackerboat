/******************************************************************************
 * Hackerboat URL fetch module
 * commands.hpp
 * This module processes incoming commands received as server responses
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef COMMAND_H
#define COMMAND_H
 
#include <curl/curl.h>
#include <jansson.h>
#include <stdlib.h>
#include <inttypes.h>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include "hal/config.h"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"

class Command {
	public:
		Command ();
		Command (BoatState &state);
		Command (BoatState &state, std::string cmd, json_t *args = NULL);
		bool attachState (BoatState &state);
		bool setCommand (std::string cmd, json_t *args = NULL);
		std::string getCmd();
		json_t *getArgs();
		bool execute ();
	private:
		std::string 	_cmd;
		json_t 			*args = NULL;
}

#endif