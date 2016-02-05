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

RESTdispatchClass::RESTdispatchClass(const char *name) {
	// set the name and calculate the hash...
	strncpy(_name, name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(_name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
}

RESTdispatchClass::RESTdispatchClass(const char *name, 
										RESTdispatchClass** table, 
										size_t tableSize) {
	// set the name and calculate the hash...
	strncpy(_name, name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(_name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
	// set the dispatch table
	_tableSize = tableSize;
	_dispatchTable = table;
}

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

uint32_t RESTdispatchClass::setName (const char *name) {
	// set the name, calculate the hash, and return the hash
	strncpy(_name, name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(_name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
	return _hash;
}

allDispatchClass::allDispatchClass(hackerboatStateClassStorable* target) {
	_name = "all";
	MurmurHash3_x86_32(_name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
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
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
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
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
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
	MurmurHash3_x86_32(_ name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
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
	json_t *out = json_object();
	int32_t insert;
	// check that we have a target and we can open the file...
	if (_target && _target->openFile()) {
		// check and see if we have a trailing number...
		if (sscanf(tokens[currentToken + 1], "%d", insert)) {
			
		} else {
		}
	}
	return NULL;
}