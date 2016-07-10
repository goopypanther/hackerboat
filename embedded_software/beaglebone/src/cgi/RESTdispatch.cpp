/******************************************************************************
 * Hackerboat Beaglebone types module
 * RESTdispatch.cpp
 * This module drives the REST interface
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include "RESTdispatch.hpp"
#include <unistd.h>
#include "logs.hpp"
#include "stateStructTypes.hpp"
#include "boneState.hpp"
#include "arduinoState.hpp"
#include "json_utilities.hpp"
#include "jansson.h"
#include "navigation.hpp"
#include "gps.hpp"


#include <string>
#include <vector>
#include <map>

static logError *errLog = logError::instance();

json_t* RESTdispatchClass::dispatch (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	// check if we've reached the end of URI...
	if (currentToken < tokens.size()) {
		// check for the token in the dispatch map
		if (_dispatchTable != NULL && _dispatchTable->count(tokens[currentToken])) {
			return (_dispatchTable->at(tokens[currentToken])).dispatch(tokens, (currentToken + 1), query, method, body);
		}
		 
		// check if the token is a number
		if (_numberDispatch != NULL) {
			size_t idx = 0;
			try {
				std::stoi(tokens[currentToken], &idx);
				if (idx == tokens[currentToken].size()) {
					return this->_numberDispatch->dispatch(tokens, currentToken, query, method, body);
				} else return NULL;
			} catch (const std::invalid_argument& ia) {
			}
		}

		// If it's not in _dispatch or a number, call the default function
		return this->defaultFunc(tokens, currentToken, query, method, body);
	} else {
		// if we've reached the end of the URI, call the root function of this object
		return this->root(tokens, currentToken, query, method, body); 
	}
}

bool RESTdispatchClass::addEntry (std::string name, RESTdispatchClass *entry) {
	// check that we got a real reference
	if (entry) {
		return (_dispatchTable->emplace(name, *entry)).second;
	} else {
		return false;
	}
}

bool RESTdispatchClass::addNumber (RESTdispatchClass *entry) {
	// if entry exists, attach it to the _numberDispatch pointer
	if (entry) {
		_numberDispatch = entry;
		return true;
	} else {
		return false;
	}
}

bool allDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* allDispatchClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	int count = _target->countRecords();
	if (count >= 0) {
		json_t* out = json_array();
		for (int i = 0; i < count; i++) {
			if (_target->getRecord(i)) {
				json_array_append_new(out, _target->pack());
			}
		}
		return out;
	} 
	return NULL;
}

bool numberDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* numberDispatchClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	size_t count;
	int val = std::stoi(tokens[currentToken], &count);
	if (count > 0) {
		if (_target->getRecord(val)) {
			return _target->pack();
		}
	}
	return NULL;
}

bool countDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* countDispatchClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	json_int_t count;
	json_t *out = json_object();
	if (!_target) return NULL;
	count = _target->countRecords();
	json_object_set(out, "count", json_integer(count));
	return out;
}

bool insertDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* insertDispatchClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	json_error_t errJSON;
	int32_t insert;
	// check that we have a target and we can open the file...
	if (_target) {
		// populate from the body...
		if (_target->parse(json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON), false)) {
			if (!_target->isValid()) {
				return NULL;
			}
			// check and see if we have a trailing number...
			size_t count;
			insert = std::stoi(tokens[currentToken + 1], &count);
			if (count > 0) {
				if (insert < _target->countRecords()) {
					if (true/*!_target->insert(count)*/) {
						if (!_target->appendRecord()) {
							return NULL;
						}
					}  
				} else if (!_target->appendRecord()) {
					return NULL;
				}
			} else if (!_target->appendRecord()) {
				return NULL;
			}
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
	return _target->pack();
}

bool appendDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* appendDispatchClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	json_error_t errJSON;
	// check that we have a target and we can open the file...
	if (_target) {
		// populate from the body...
		if (_target->parse(json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON), false)) {
			if (_target->isValid()) {
				if (!_target->appendRecord()) {
					return NULL;
				}
			} else {
				return NULL;
			}
		} 
	} else {
		return NULL;
	}
	return _target->pack();
}

json_t* rootRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	return NULL;
}

json_t* rootRESTClass::defaultFunc (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	return this->root(tokens, currentToken, query, method, body);
}

json_t* boneStateRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	if (_target->getLastRecord()) {
		return _target->pack();
	}
	return NULL;
}

bool boneStateRESTClass::setTarget(boneStateClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* boneStateRESTClass::defaultFunc (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (tokens[currentToken] == "command") {
		return command(body);
	} else if (tokens[currentToken] == "waypointNext") {
		return waypointNext(body);
	} else if (tokens[currentToken] == "waypointStrength") {
		return waypointStrength(body);
	} else if (tokens[currentToken] == "waypointStrengthMax") {
		return waypointStrengthMax(body);
	} else if (tokens[currentToken] == "waypointAccuracy") {
		return waypointAccuracy(body);
	} else if (tokens[currentToken] == "autonomous") {
		return autonomous(body);
	} else {
		_target->writeRecord();
		return NULL;
	}
}	

json_t* boneStateRESTClass::command(std::string body) {
	std::string command;
	json_t *input; 
	json_t obj;
	json_error_t errJSON;
	boatModeEnum cmd;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// check that the load went well, load in the the last state vector, 
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) || 
		(json_unpack(input, "{s:o}", "command", &obj))) {
		free(input);
		return NULL;
	}
	if (!::parse(&obj, &command)) {
		free(input);
		return NULL;
	}
	free(input);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	// parse the incoming state
	if (boneStateClass::modeNames.get(command, &cmd)) {
		if(_target->setCommand(cmd)) {
			if (!_target->writeRecord()) return NULL;
			return _target->pack();
		} 
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointNext(std::string body) {
	json_t *input;
	json_t obj;
	json_error_t errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointNext", &obj))) {
		free(input);
		return NULL;
	}
	if (!::parse(&obj, &(_target->waypointNext))) {
		free(input);
		return NULL;
	}
	free(input);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointStrength(std::string body) {
	json_t *input;
	json_t obj;
	json_error_t errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointStrength", &obj))) {
		free(input);
		
		return NULL;
	}
	if (!::parse(&obj, &(_target->waypointStrength))) {
		free(input);
		return NULL;
	}
	free(input);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointStrengthMax(std::string body) {
	json_t *input;
	json_t obj;
	json_error_t errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointStrengthMax", &obj))) {
		free(input);
		return NULL;
	}
	if (!::parse(&obj, &(_target->waypointStrengthMax))) {
		free(input);
		return NULL;
	}
	free(input);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointAccuracy(std::string body) {
	json_t *input, *val;
	json_error_t errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	val = json_object_get(input, "waypointAccuracy");
	if ((!input) || (!_target->getLastRecord()) || val) {
		json_decref(input);
		json_decref(val);
		return NULL;
	}
	if (!::parse(val, &(_target->waypointAccuracy))) {
		json_decref(input);
		json_decref(val);
		return NULL;
	}
	json_decref(input);
	json_decref(val);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::autonomous(std::string body) {
	json_t *input, *val;
	json_error_t errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, &errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	val = json_object_get(input, "autonomous");
	if ((!input) || (!_target->getLastRecord()) || val) {
		json_decref(input);
		json_decref(val);
	}
	if (!::parse(val, &(_target->autonomous))) {
		json_decref(input);
		json_decref(val);
		return NULL;
	}
	json_decref(input);
	json_decref(val);
	
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* gpsRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	if (_target->getLastRecord()) {
		return _target->pack();
	}
	return NULL;
}

bool gpsRESTClass::setTarget(gpsFixClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* waypointRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	if (_target->getLastRecord()) {
		return _target->pack();
	}
	return NULL;
}

bool waypointRESTClass::setTarget(waypointClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* navRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	if (_target->getLastRecord()) {
		return _target->pack();
	}
	return NULL;
}

bool navRESTClass::setTarget(navClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* arduinoStateRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	if (!_target) return NULL;
	if (_target->getLastRecord()) {
		return _target->pack();
	}
	return NULL;
}

bool arduinoStateRESTClass::setTarget(arduinoStateClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* resetArduinoRest::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	json_t *retval = json_object();
	system("config-pin P9.15 low");		// set the Arduino reset pin low
	usleep(100000);						// sleep for 100 milliseconds
	system("config pin P9.15 high");	// set the Arduino reset pin high
	json_object_set_new(retval, "success", json_boolean(true));
	return retval;
}

json_t* arduinoRESTClass::root (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
	return this->defaultFunc(tokens, currentToken, query, method, body);
}

bool arduinoRESTClass::setTarget(arduinoStateClass* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* arduinoRESTClass::defaultFunc (std::vector<std::string> tokens, uint32_t currentToken, std::string query, httpMethod method, std::string body) {
		
	// write out the incoming URI to the Arduino & read the response
	return _target->writeArduino(tokens[currentToken], query);
}

RESTdispatchClass::httpMethod RESTdispatchClass::methodFromString(const char *m)
{
	if (!m)
		return httpMethod::UNKNOWN;
	else if (0 == strcasecmp(m, "GET"))
		return httpMethod::GET;
	else if (0 == strcasecmp(m, "POST"))
		return httpMethod::POST;
	else if (0 == strcasecmp(m, "PUT"))
		return httpMethod::PUT;
	else if (0 == strcasecmp(m, "DELETE"))
		return httpMethod::DELETE;
	else
		return httpMethod::UNKNOWN;
}

namespace std {
	using httpMethod = RESTdispatchClass::httpMethod;
	string to_string(httpMethod m)
	{
		switch (m) {
		default: return "UNKNOWN";
		case httpMethod::GET: return "GET";
		case httpMethod::PUT: return "PUT";
		case httpMethod::POST: return "POST";
		case httpMethod::DELETE: return "DELETE";
		}
	}
};

