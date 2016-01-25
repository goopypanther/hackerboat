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
		virtual RESTdispatchClass(void);
		virtual RESTdispatchClass(const char *_name);	/**< Create a dispatch object with _name */
		virtual RESTdispatchClass(const char *_name, RESTdispatchClass** _table, size_t _tableSize); /**< Create a dispatch object with _name and the given dispatch table */
		
		virtual json_t* dispatch (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen); /**< Dispatch on the tokenized (and hashed) URI */
		virtual bool addEntry (RESTdispatchClass *entry);	/**< Add an entry to the dispatch table */
		virtual bool addNumber (RESTdispatchClass *entry);	/**< Add an entry to call when the next token is a number */
		uint32_t setName (char *name);				/**< Set the name of the object */
		bool match (uint32_t hash);					/**< Check if this object matches the given hash */
		virtual json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);			/**< Function to execute if this is the last token */
		virtual json_t*	defaultFunc (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen); 	/**< Function to execute if the next token doesn't match anything in the dispatch table */
		
	private:	
		RESTdispatchClass* _defaultDispatch;
		RESTdispatchClass* _numberDispatch;
		RESTdispatchClass** _dispatchTable;
		size_t tableSize
		uint32_t _hash;
		char name[MAX_TOKEN_LEN];
};

class allDispatchClass : public RESTdispatchClass {
	public:
		allDispatchClass(const char *_name, hackerboatStateClassStorable* target);	/**< Create an 'all' dispatch item attached to hackerboatStateClassStorable target*/
		bool setTarget (hackerboatStateClassStorable* target);						/**< Set the target hackerboatStateClassStorable */
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		
	private:
		hackerboatStateClassStorable* _target;
};

class numberDispatchClass : public RESTdispatchClass {
	public:
		numberDispatchClass(hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class countDispatchClass : public RESTdispatchClass {
	public:
		countDispatchClass(const char *_name, hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class insertDispatchClass : public RESTdispatchClass {
	public:
		insertDispatchClass(const char *_name, hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class appendDispatchClass : public RESTdispatchClass {
	public:
		insertDispatchClass(const char *_name, hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class rootRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class boneStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (hackerboatStateClassStorable* target);
		json_t*	defaultFunc (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);

	private:
		json_t*	command (char* body, int bodyLen);
		json_t*	waypointNext (char* body, int bodyLen);
		json_t*	waypointStrength (char* body, int bodyLen);
		json_t*	waypointStrengthMax (char* body, int bodyLen);
		json_t*	waypointAccuracy (char* body, int bodyLen);
		json_t*	offshore (char* body, int bodyLen);
		hackerboatStateClassStorable* _target;
};

class gpsRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class waypointRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class navRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class arduinoStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class resetArduinoRest : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class arduinoRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc  (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};
