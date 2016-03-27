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
#include <BlackUART/BlackUART.h>
#include <BlackGPIO/BlackGPIO.h>
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

using namespace BlackLib;

static logError *errLog = logError::instance();

json_t* RESTdispatchClass::dispatch (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	// check if we've reached the end of URI...
	if (currentToken < tokens.size()) {
		// check for the token in the dispatch map
		if (_dispatchTable->count(tokens[currentToken])) {
			return (_dispatchTable->at(tokens[currentToken])).dispatch(tokens, (currentToken + 1), query, method, body);
		}
		 
		// check if the token is a number; if not, call the default function
		size_t idx = 0;
		int num = std::stoi(tokens[currentToken], &idx);
		if (idx) {
			return this->_numberDispatch->dispatch(tokens, currentToken, query, method, body);
		} else {
			return this->defaultFunc(tokens, currentToken, query, method, body);
		}
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

json_t* allDispatchClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	json_t* out = json_array();
	if (!_target) return NULL;
	int count = _target->countRecords();
	if (count > 0) {
		for (int i; i < count; i++) {
			_target->getRecord(i);
			json_array_append_new(out, _target->pack());
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

json_t* numberDispatchClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* countDispatchClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* insertDispatchClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	json_error_t *errJSON;
	int32_t insert;
	// check that we have a target and we can open the file...
	if (_target) {
		// populate from the body...
		if (_target->parse(json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON), false)) {
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

json_t* appendDispatchClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	json_error_t *errJSON;
	// check that we have a target and we can open the file...
	if (_target) {
		// populate from the body...
		if (_target->parse(json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON), false)) {
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

json_t* rootRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	return NULL;
}

json_t* rootRESTClass::defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	return this->root(tokens, currentToken, query, method, body);
}

json_t* boneStateRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* boneStateRESTClass::defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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
	json_t *input, *obj;
	json_error_t *errJSON;
	boatModeEnum cmd;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// check that the load went well, load in the the last state vector, 
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) || 
		(json_unpack(input, "{s:o}", "command", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &command)) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
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
	json_t *input, *obj;
	json_error_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointNext", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &(_target->waypointNext))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointStrength(std::string body) {
	json_t *input, *obj;
	json_error_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointStrength", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &(_target->waypointStrength))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointStrengthMax(std::string body) {
	json_t *input, *obj;
	json_error_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointStrengthMax", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &(_target->waypointStrengthMax))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::waypointAccuracy(std::string body) {
	json_t *input, *obj;
	json_error_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "waypointAccuracy", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &(_target->waypointAccuracy))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* boneStateRESTClass::autonomous(std::string body) {
	json_t *input, *obj;
	json_error_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if (!_target) return NULL;
	// load in incoming request body
	input = json_loadb(body.c_str(), body.length(), JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getLastRecord()) ||  
		(json_unpack(input, "{s:o}", "autonomous", obj))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	if (!::parse(obj, &(_target->autonomous))) {
		free(obj);
		free(input);
		free(errJSON);
		return NULL;
	}
	free(obj);
	free(input);
	free(errJSON);
	clock_gettime(CLOCK_REALTIME, &(_target->lastContact));
	if (_target->isValid()) {
		if (_target->writeRecord()) return NULL;
		return _target->pack();
	} 
	return NULL;
}

json_t* gpsRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* waypointRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* navRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* arduinoStateRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
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

json_t* resetArduinoRest::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	BlackGPIO	ardResetPin(ARDUINO_RESET_PIN, output, FastMode);
	if (ardResetPin.setValue(low)) {
		usleep(100000);	// sleep for 100 milliseconds
		if (ardResetPin.setValue(high)) {
			return json_pack("{sb}", "success", true);
		} else {
			return json_pack("{sb}", "success", false);
		}
	} else {
		return json_pack("{sb}", "success", false);
	}
}

json_t* arduinoRESTClass::root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	return this->defaultFunc(tokens, currentToken, query, method, body);
}

json_t* arduinoRESTClass::defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body) {
	json_t *out, *in;
	json_error_t *errIn;
	BlackUART port(ARDUINO_REST_UART, ARDUINO_BAUD, ParityNo, StopOne, Char8);
	std::string buf;
	uint32_t cnt = 0;
	
	// attempt to open the serial port
	port.setReadBufferSize(LOCAL_BUF_LEN);
	while (cnt < UART_TIMEOUT) {
		if (port.open(ReadWrite)) break;
		usleep(1);
		cnt++;
	}
	
	// if we timed out, return a NULL
	if (cnt >= UART_TIMEOUT) {
		errLog->write("REST Arduino Serial", "Failed to open serial port for write");
		port.close();
		return NULL;
	}
	
	// write out the incoming URI to the Arduino

	for (uint8_t i = (currentToken + 1); i < tokens.size(); i++) {
		port.write("/");
		port.write(tokens[i].c_str());
	}
	port.write("?");
	port.write(query.c_str());
	port.write("\r\n");
	
	// read the incoming buffer
	port >> buf;
	port.close();
	if (buf != BlackLib::UART_READ_FAILED) {
		in = json_loadb(buf.c_str(), buf.length(), JSON_DECODE_ANY, errIn);
		if (in) {
			free(errIn);
			return in;
		} else {
			json_decref(in);
			free(errIn);
			errLog->write("REST Arduino Serial", "Failed to parse incoming json from Arduino");
			return NULL;
		}
	} else {
		errLog->write("REST Arduino Serial", "Failed to read return value");
		return NULL;
	}
}
