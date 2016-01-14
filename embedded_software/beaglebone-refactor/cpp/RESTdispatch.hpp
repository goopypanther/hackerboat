/******************************************************************************
 * Hackerboat Beaglebone types module
 * RESTdispatch.hpp
 * This module drives the REST interface
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>
#include "stateStructTypes.h"
#include "config.h"

class RESTdispatchClass {
	public:
		RESTdispatchClass(void);
		RESTdispatchClass(char *_name);
		RESTdispatchClass(char *_name, RESTdispatchClass** _table, size_t _tableSize);
		
		json_t* dispatch (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
		bool addEntry (RESTdispatchClass *entry);
		bool addDefault (RESTdispatchClass *entry);
		uint32_t setName (char *name);
		virtual json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
		
		char 		name[MAX_TOKEN_LEN];
		uint32_t 	hash;
		
	private:	
		RESTdispatchClass** dispatchTable;
		size_t tableSize
}json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);

class boneStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class gpsRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class waypointRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class navRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class arduinoStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class resetArduinoRest : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}

class arduinoRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* body, int bodyLen);
}
