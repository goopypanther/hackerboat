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

RESTdispatchClass::RESTdispatchClass(const char *_name) {
	strncpy(name, _name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
}

RESTdispatchClass::RESTdispatchClass(const char *_name, 
										RESTdispatchClass** _table, 
										size_t _tableSize) {
	strncpy(name, _name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
	tableSize = _tableSize;
	_dispatchTable = _table;
}

json_t* RESTdispatchClass::dispatch (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen) {
	int num;
	if (currentToken < tokenCnt) {
		for (uint8_t i = 0; i < _tableSize; i++) {
			if (_dispatchTable[i]->match(tokenHashes[currentToken])) {
				return _dispatchTable[i]->dispatch(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
			}
		}
		if (sscanf(tokens[currentToken], "%d", &num) {
			if (this->_numberDispatch) {
				return this->_numberDispatch->root(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
			} else return NULL;
		} else {
			return this->defaultFunc(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken + 1, query, method, body, bodyLen);
		}
	} else {
		return this->root(tokens, tokenHashes, tokenLengths, tokenCnt, currentToken, query, method, body, bodyLen); 
	}
}

bool RESTdispatchClass::addEntry (RESTdispatchClass *entry) {
	RESTdispatchClass **temp;
	if (entry) {
		temp = (RESTdispatchClass**)calloc((tableSize + 1), sizeof(RESTdispatchClass*));
		if (temp) {
			for (int i = 0; i < tableSize; i++) {
				temp[i] = _dispatchTable[i];
			}
			temp[tableSize] = entry;
			free(_dispatchTable);
			_dispatchTable = temp;
			tableSize++;
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool RESTdispatchClass::addNumber (RESTdispatchClass *entry) {
	if (entry) {
		_numberDispatch = entry;
		return true;
	} else {
		return false;
	}
}

uint32_t RESTdispatchClass::setName (const char *name) {
	strncpy(name, _name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
	return _hash;
}

allDispatchClass::allDispatchClass(const char *_name, hackerboatStateClassStorable* target) {
	strncpy(name, _name, MAX_TOKEN_LEN);
	MurmurHash3_x86_32(name, strnlen(_name, MAX_TOKEN_LEN), HASHSEED, &_hash);
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
			return NULL;
		}
	} else {
		return NULL;
	}
}

numberDispatchClass::numberDispatchClass(const char *_name, hackerboatStateClassStorable* target) {
	strncpy(name, _name, MAX_TOKEN_LEN);
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


countDispatchClass::countDispatchClass(const char *_name, hackerboatStateClassStorable* target) {
	strncpy(name, _name, MAX_TOKEN_LEN);
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