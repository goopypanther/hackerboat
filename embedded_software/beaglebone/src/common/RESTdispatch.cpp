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
#include "MurmurHash3.h"
#include <BlackGPIO.h>
#include <unistd.h>

#include <string>

using namespace string;
using namespace BlackLib;

json_t* RESTdispatchClass::dispatch (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	int num;
	// check if we've reached the end of URI...
	if (currentToken < tokenCnt) {
		// iterate through the dispatch table, looking for a match
		for (uint8_t i = 0; i < _tableSize; i++) {
			if (_dispatchTable[i]->match(tokenHashes[currentToken])) {
				// if we find a match, dispatch that object
				return _dispatchTable[i]->dispatch(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
			}
		}
		// check if the token is a number
		if (sscanf(tokens[currentToken], "%d", &num) {
			// check that _numberDispatch points to an object, then call its dispatch function
			if (this->_numberDispatch) {
				return this->_numberDispatch->dispatch(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
			}
		// otherwise, call the default function
		} else {
			return this->defaultFunc(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
		}
	} else {
		// if we've reached the end of the URI, call the root function of this object
		return this->root(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken, query, method, body, bodyLen); 
	}
}

bool RESTdispatchClass::addEntry (RESTdispatchClass *entry) {
	RESTdispatchClass **temp;
	// check that we got a real reference
	if (entry) {
		// make a bigger table
		temp = (RESTdispatchClass**)calloc((_tableSize + 1), sizeof(RESTdispatchClass*));
		// if we succeeded, copy everything over and add the latest entry to the end
		if (temp) {
			for (int i = 0; i < _tableSize; i++) {
				temp[i] = _dispatchTable[i];
			}
			temp[_tableSize] = entry;
			free(_dispatchTable);
			_dispatchTable = temp;
			_tableSize++;
			return true;
		} else {
			return false;
		}
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

uint32_t RESTdispatchClass::setName (const string name) {
	// set the name, calculate the hash, and return the hash
	_name = name;
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	return _hash;
}

allDispatchClass::allDispatchClass(hackerboatStateClassStorable* target) {
	_name = "all";
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	_target = target;
}

bool allDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* allDispatchClass::root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	int32_t count;
	json_t* out = json_array();
	if (!_target) return NULL;
	if (_target->openFile()) {
		count = _target->count();
		if (count > 0) {
			for (int32_t i; i < count; i++) {
				_target->getRecord(i);
				json_array_append_new(out, _target->pack());
			}
			_target->closeFile();
			return out;
		} else {
			_target->closeFile();
		}
	}
	return NULL;
}

numberDispatchClass::numberDispatchClass(hackerboatStateClassStorable* target) {
	_name = "";
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	_target = target;
}

bool numberDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* numberDispatchClass::root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	int32_t count;
	if (!_target) return NULL;
	if (sscanf(tokens[currentToken], "%d", &count)) {
		if (_target->openFile()) {
			if (_target->getRecord(count)) {
				_target->closeFile();
				return _target->pack();
			}
			_target->closeFile();
		}
	}
	return NULL;
}

countDispatchClass::countDispatchClass(hackerboatStateClassStorable* target) {
	name = "count";
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	_target = target;
}

bool countDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* countDispatchClass::root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	json_int_t count;
	json_t *out = json_object();
	if (!_target) return NULL;
	if (_target->openFile()) {
		count = _target->count();
		json_object_set(out, "count", json_integer(count));
		_target->closeFile();
		return out;
	}
	return NULL;
}

insertDispatchClass::insertDispatchClass(hackerboatStateClassStorable* target) {
	name = "insert";
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	_target = target;
}

bool insertDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* insertDispatchClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	json_err_t *errJSON;
	int32_t insert;
	// check that we have a target and we can open the file...
	if (_target && _target->openFile()) {
		// populate from the body...
		if (_target->parse(json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON))) {
			if (!_target->isValid()) {
				return NULL;
			}
			// check and see if we have a trailing number...
			if (sscanf(tokens[currentToken + 1], "%d", insert)) {
				if (insert < _target->count()) {
					if (!_target->insert(count)) {
						_target->closeFile();
						return NULL;
					}
				} else if (!_target->append()) {
					_target->closeFile();
					return NULL;
				}
			} else if (!_target->append()) {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return errJSON;
		}
	} else {
		_target->closeFile();
		return NULL;
	}
	_target->closeFile();
	return _target->pack();
}

appendDispatchClass::appendDispatchClass(hackerboatStateClassStorable* target) {
	name = "append";
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	_target = target;
}

bool appendDispatchClass::setTarget (hackerboatStateClassStorable* target) {
	if (target) {
		_target = target;
		return true;
	} else {
		return false;
	}
}

json_t* appendDispatchClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	json_err_t *errJSON;
	// check that we have a target and we can open the file...
	if (_target && _target->openFile()) {
		// populate from the body...
		if (_target->parse(json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON))) {
			if (_target->isValid()) {
				if (!_target->append()) {
					_target->closeFile();
					return NULL;
				}
			} else {
				_target->closeFile();
				return errJSON;
			}
		} 
	} else {
		return NULL;
	}
	_target->closeFile();
	return _target->pack();
}

json_t* rootRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	return NULL;
}

json_t* rootRESTClass::defaultFunc(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	return this->root(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken, query, method, body, bodyLen);
}

boneStateRESTClass::boneStateRESTClass(const string name) {
	// set the name and calculate the hash...
	_name = name
	MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
	setHashes();
}

json_t* boneStateRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (!_target->openFile()) return NULL;
	if (_target->getRecord(_target->count())) {
		_target->closeFile():
		return _target->pack();
	}
	_target->closeFile():
	return NULL;
}

bool boneStateRESTClass::setTarget(boneStateClass* target) {
	if (target) {
		_target = target;
		setHashes();
		return true;
	} else {
		return false;
	}
}

json_t* boneStateRESTClass::defaultFunc(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (_target) clock_gettime(CLOCK_REALTIME &(_target->lastContact));
	switch (tokenHashes[currentToken]) {
		case (commandHash):
			return command(body, bodyLen);
		case (waypointNextHash):
			return waypointNext(body, bodyLen);
		case (waypointStrengthHash):
			return waypointStrength(body, bodyLen);
		case (waypointStrengthMaxHash):
			return waypointStrengthMax(body, bodyLen);
		case (waypointAccuracyHash):
			return waypointAccuracy(body, bodyLen);
		case (autonomousHash):
			return autonomous(body, bodyLen);
		default:
			return NULL;
	}
}	

void boneStateRESTClass::setHashes(void) {
	MurmurHash3_x86_32("command", strnlen("command", MAX_TOKEN_LEN), HASHSEED, &commandHash);
	MurmurHash3_x86_32("waypointNext", strnlen("waypointNext", MAX_TOKEN_LEN), HASHSEED, &waypointNextHash);
	MurmurHash3_x86_32("waypointStrength", strnlen("waypointStrength", MAX_TOKEN_LEN), HASHSEED, &waypointStrengthHash);
	MurmurHash3_x86_32("waypointStrengthMax", strnlen("waypointStrengthMax", MAX_TOKEN_LEN), HASHSEED, &waypointStrengthMaxHash);
	MurmurHash3_x86_32("waypointAccuracy", strnlen("waypointAccuracy", MAX_TOKEN_LEN), HASHSEED, &waypointAccuracyHash);
	MurmurHash3_x86_32("autonomous", strnlen("autonomous", MAX_TOKEN_LEN), HASHSEED, &autonomousHash);
}

json_t* boneStateRESTClass::command(char* body, int bodyLen) {
	uint32_t hash;
	char command[MAX_TOKEN_LEN];
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// check that the load went well, load in the the last state vector, 
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((!input) || (!_target->getRecord(_target->count())) || 
		(json_unpack(input, "{s:s}", "command", command))) {
		free(input);
		free(errJSON);
		return NULL;
	}
	// hash the incoming command for comparison
	MurmurHash3_x86_32(command, strlen(command), HASHSEED, &hash);
	// iterate over the available states to see if we have a match
	for (int8_t i = 0; i < boneStateCount; i++) {
		if (_target->stateHashes[i] == hash) {
			this->setCommand(i);
			free(input);
			free(errJSON);
			// write to the database (return NULL if failed)
			if (!_target->writeRecord()) return NULL;
			_target->closeFile();
			return _target->pack();
		}
	}
	free(input);
	free(errJSON);
	return NULL;
}

json_t* boneStateRESTClass::waypointNext(char* body, int bodyLen) {
	uint32_t hash;
	int32_t waypoint;
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((input) && (_target->getrecord(_target->count()))) { 
		json_unpack(input, "{s:d}", "waypointNext", &waypoint);
		_target->waypointNext = waypoint;
		free(input);
		free(errJSON);
		if (_target->isValid()) {
			if (_target->writeRecord()) {
				_target->closeFile();
				return _target->pack();
			} else {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return _target->pack();
		}
	} else {
		free(input);
		free(errJSON);
		_target->closeFile();
		return NULL;
	}
}

json_t* boneStateRESTClass::waypointStrength(char* body, int bodyLen) {
	uint32_t hash;
	double strength;
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((input) && (_target->getrecord(_target->count()))) { 
		json_unpack(input, "{s:f}", "waypointStrength", &strength);
		_target->waypointStrength = strength;
		free(input);
		free(errJSON);
		if (_target->isValid()) {
			if (_target->writeRecord()) {
				_target->closeFile();
				return _target->pack();
			} else {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return _target->pack();
		}
	} else {
		free(input);
		free(errJSON);
		_target->closeFile();
		return NULL;
	}
}

json_t* boneStateRESTClass::waypointStrengthMax(char* body, int bodyLen) {
	uint32_t hash;
	double strength;
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((input) && (_target->getrecord(_target->count()))) { 
		json_unpack(input, "{s:f}", "waypointStrengthMax", &strength);
		_target->waypointStrengthMax = strength;
		free(input);
		free(errJSON);
		if (_target->isValid()) {
			if (_target->writeRecord()) {
				_target->closeFile();
				return _target->pack();
			} else {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return _target->pack();
		}
	} else {
		free(input);
		free(errJSON);
		_target->closeFile();
		return NULL;
	}
}

json_t* boneStateRESTClass::waypointAccuracy(char* body, int bodyLen) {
	uint32_t hash;
	double accuracy;
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((input) && (_target->getrecord(_target->count()))) { 
		json_unpack(input, "{s:f}", "waypointAccuracy", &accuracy);
		_target->waypointAccuracy = accuracy;
		free(input);
		free(errJSON);
		if (_target->isValid()) {
			if (_target->writeRecord()) {
				_target->closeFile();
				return _target->pack();
			} else {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return _target->pack();
		}
	} else {
		free(input);
		free(errJSON);
		_target->closeFile();
		return NULL;
	}
}

json_t* boneStateRESTClass::autonomous(char* body, int bodyLen) {
	uint32_t hash;
	bool autonomous;
	json_t *input;
	json_err_t *errJSON;
	
	// make sure that _target is non-NULL and we can open the target file
	if ((!_target) || (!_target->openFile())) return NULL;
	// load in incoming request body
	input = json_loadb(body, bodyLen, JSON_DECODE_ANY, errJSON);
	// and see if we got a correctly formatted JSON object... otherwise, return NULL 
	if ((input) && (_target->getrecord(_target->count()))) { 
		json_unpack(input, "{s:b}", "autonomous", &autonomous);
		_target->autonomous = autonomous;
		free(input);
		free(errJSON);
		if (_target->isValid()) {
			if (_target->writeRecord()) {
				_target->closeFile();
				return _target->pack();
			} else {
				_target->closeFile();
				return NULL;
			}
		} else {
			_target->closeFile();
			return _target->pack();
		}
	} else {
		free(input);
		free(errJSON);
		_target->closeFile();
		return NULL;
	}
}

json_t* gpsRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (!_target->openFile()) return NULL;
	if (_target->getRecord(_target->count())) {
		_target->closeFile():
		return _target->pack();
	}
	_target->closeFile():
	return NULL;
}

bool gpsRESTClass::setTarget(gpsFixClass* target) {
	if (target) {
		_target = target;
		setHashes();
		return true;
	} else {
		return false;
	}
}

json_t* waypointRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (!_target->openFile()) return NULL;
	if (_target->getRecord(_target->count())) {
		_target->closeFile():
		return _target->pack();
	}
	_target->closeFile():
	return NULL;
}

bool waypointRESTClass::setTarget(waypointClass* target) {
	if (target) {
		_target = target;
		setHashes();
		return true;
	} else {
		return false;
	}
}

json_t* navRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (!_target->openFile()) return NULL;
	if (_target->getRecord(_target->count())) {
		_target->closeFile():
		return _target->pack();
	}
	_target->closeFile():
	return NULL;
}

bool navRESTClass::setTarget(navClass* target) {
	if (target) {
		_target = target;
		setHashes();
		return true;
	} else {
		return false;
	}
}

json_t* arduinoStateRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	if (!_target->openFile()) return NULL;
	if (_target->getRecord(_target->count())) {
		_target->closeFile():
		return _target->pack();
	}
	_target->closeFile():
	return NULL;
}

bool arduinoStateRESTClass::setTarget(arduinoStateClass* target) {
	if (target) {
		_target = target;
		setHashes();
		return true;
	} else {
		return false;
	}
}

json_t* resetArduinoRest::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	BlackGPIO	ardResetPin(ARDUINO_RESET_PIN, output, FastMode);
	if (ardResetPin.setValue(low)) {
		usleep(100000);	// sleep for 100 milliseconds
		if (ardResetPin.setValue(high)) {
			return json_pack{"{sb}", "success", true);
		} else {
			return json_pack{"{sb}", "success", false);
		}
	} else {
		return json_pack{"{sb}", "success", false);
	}
}

json_t* arduinoRESTClass::root(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	return this->defaultFunc(tokens, tokenHashes, tokenLength, tokenCnt, currentToken, query, method, body, bodyLen);
}

json_t* arduinoRESTClass::defaultFunc(char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	json_t* out, in;
	BlackUART port(ARDUINO_REST_UART, ARDUINO_BAUD, ParityNo, StopOne, Char8);
	uint32_t cnt = 0;
	char buf[LOCAL_BUF_LEN] = {0};
	
	// attempt to open the serial port
	while (cnt < UART_TIMEOUT) {
		if (port.open(ReadWrite)) break;
		usleep(1);
		cnt++;
	}
	
	// if we timed out, return a NULL
	if (cnt >= UART_TIMEOUT) {
		port.close();
		return NULL;
	}
	
	// write out the incoming URI to the Arduino
	port.write("/", 1);
	for (uint8_t i = (currentToken + 1); i < tokenCnt; i++) {
		port.write(tokens[i], tokenLength[i]);
		port.write("/", 1);
	}
	
	// read the incoming buffer
	if (port.read(buf, LOCAL_BUF_LEN)) {
		return json_loadb(buf, LOCAL_BUF_LEN, 0);
	} else {
		return NULL;
	}
}
