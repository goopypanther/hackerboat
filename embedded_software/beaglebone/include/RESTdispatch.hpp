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

#ifndef RESTDISPATCH_H
#define RESTDISPATCH_H
 
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>
#include "stateStructTypes.hpp"
#include "config.h"

#include <string>

class gpsFixClass;
class navClass;
class boneStateClass;
class arduinoStateClass;

using string = std::string;

/**
 * @class RESTDispatchClass
 *
 * @brief This is the base class for nodes on the REST call tree
 * 
 * When a REST request arrives, the URI is split into tokens on the slashes and each token is hashed.
 * A top level object of this type is passed an array of tokens, an array of hashes, the index of the 
 * current token, and the contents of the REST request (query string, method, and POST body, if any) 
 * through its dispatch() method. If there are more tokens beyond the current one, it sorts through 
 * the dispatch list and calls the dispatch() method of the matching object.  
 *
 */

class RESTdispatchClass {
	public:
		RESTdispatchClass(void) = default;
		RESTdispatchClass(const string name) : _name(name) {
			MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);
		};		/**< Create a dispatch object with name */
		/**
		 * @brief Constructor for a dispatch object with name and given dispatch table
		 */
		RESTdispatchClass	(const string name, 		/**< Name of the object */
									RESTdispatchClass** table,	/**< Table of leaf nodes to dispatch on */
									size_t tableSize) : 		/**< Number of items in the dispatch table */
									_name(name),
									_dispatchTable(table),
									_tableSize(tableSize) {
			MurmurHash3_x86_32(_name.c_str(), _name.length(), HASHSEED, &_hash);									
		};
									
		virtual bool addEntry (RESTdispatchClass *entry);		/**< Add an entry to the dispatch table */
		virtual bool addNumber (RESTdispatchClass *entry);		/**< Add an entry to call when the next token is a decimal number */
		virtual uint32_t setName (const string name);			/**< Set the name of the object */
		bool match (uint32_t hash) {return (hash == _hash);};	/**< Check if this object matches the given hash */
		/**
		 * @brief Dispatch on the token designated by currentToken
		 */
		virtual json_t* dispatch 	(char** tokens, 			/**< Array of tokens from the URI request */
									uint32_t* tokenHashes, 		/**< Array of hashes of each token */
									size_t* tokenLengths, 		/**< The length of each token */
									int tokenCnt, 				/**< Number of tokens */
									int currentToken, 			/**< Token to dispatch on */
									char* query, 				/**< Query string, if any, from the request */
									char* method, 				/**< Request method, generally either GET or POST */
									char* body, 				/**< POST request body, if any */
									int bodyLen); 				/**< Length of the body */
		/**
		 * @brief If there are no more tokens, dispatch() calls this method. Default implementation returns NULL.
		 */							
		virtual json_t* root 		(char** tokens, 			/**< Array of tokens from the URI request */
									uint32_t* tokenHashes, 		/**< Array of hashes of each token */
									size_t* tokenLengths, 		/**< The length of each token */
									int tokenCnt, 				/**< Number of tokens */
									int currentToken, 			/**< Token to dispatch on */
									char* query, 				/**< Query string, if any, from the request */
									char* method, 				/**< Request method, generally either GET or POST */
									char* body, 				/**< POST request body, if any */
									int bodyLen); 				/**< Length of the body */
									{return NULL;};		
		/**
		 * @brief If there are no more tokens, dispatch() calls this method. Default implementation returns NULL.
		 */														
		virtual json_t*	defaultFunc (char** tokens, 			/**< Array of tokens from the URI request */
									uint32_t* tokenHashes, 		/**< Array of hashes of each token */
									size_t* tokenLengths, 		/**< The length of each token */
									int tokenCnt, 				/**< Number of tokens */
									int currentToken, 			/**< Token to dispatch on */
									char* query, 				/**< Query string, if any, from the request */
									char* method, 				/**< Request method, generally either GET or POST */
									char* body, 				/**< POST request body, if any */
									int bodyLen); 				/**< Length of the body */
									{return NULL;}; 
		
	protected:	
		RESTdispatchClass* _numberDispatch	= NULL;				/**< Object to dispatch to if the next token is a number */
		RESTdispatchClass** _dispatchTable	= NULL;				/**< Dispatch table */
		size_t _tableSize 					= 0;				/**< Size of the dispatch table */
		string _name				 		= "";				/**< Name of this object */
		uint32_t _hash 						= 0;				/**< MurmurHash3 of the object name */
};

/** 
 * @class allDispatchClass
 *
 * @brief This class is for nodes where the last token is 'all'. It is strictly a leaf node.
 *
 */
 
class allDispatchClass : public RESTdispatchClass {
	public:
		allDispatchClass(hackerboatStateClassStorable* target);		/**< Create an 'all' dispatch item attached to hackerboatStateClassStorable target*/
		bool setTarget (hackerboatStateClassStorable* target);		/**< Set the target hackerboatStateClassStorable */
		uint32_t setName (const string name) {return 0;};
		bool addEntry (RESTdispatchClass *entry) {return false;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		
	private:
		hackerboatStateClassStorable* _target;
};

/** 
 * @class numberDispatchClass
 *
 * @brief This class is for nodes where the token is a decimal number. It may have branch nodes. 
 *
 */
 
class numberDispatchClass : public RESTdispatchClass {
	public:
		numberDispatchClass(hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		uint32_t setName (const string name) {return 0;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		
	private:
		hackerboatStateClassStorable* _target;
};

class countDispatchClass : public RESTdispatchClass {
	public:
		countDispatchClass(hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class insertDispatchClass : public RESTdispatchClass {
	public:
		insertDispatchClass(hackerboatStateClassStorable* target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false};
		bool addNumber (RESTdispatchClass *entry) {return false};
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
	private:
		hackerboatStateClassStorable* _target;
};

class appendDispatchClass : public RESTdispatchClass {
	public:
		appendDispatchClass(hackerboatStateClassStorable* target);
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
		boneStateRESTClass(void) {setHashes();};
		boneStateRESTClass(const string name);			/**< Create a dispatch object with name */
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (boneStateClass* target);
		json_t*	defaultFunc (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
											
	private:
		void setHashes (void);
		json_t*	command (char* body, int bodyLen);
		json_t*	waypointNext (char* body, int bodyLen);
		json_t*	waypointStrength (char* body, int bodyLen);
		json_t*	waypointStrengthMax (char* body, int bodyLen);
		json_t*	waypointAccuracy (char* body, int bodyLen);
		json_t*	autonomous (char* body, int bodyLen);
		boneStateClass* _target = NULL;
		static uint32_t commandHash = -1;
		static uint32_t waypointNextHash = -1;
		static uint32_t waypointStrengthHash = -1;
		static uint32_t waypointStrengthMaxHash = -1;
		static uint32_t waypointAccuracyHash = -1;
		static uint32_t autonomousHash = -1;
};

class gpsRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (gpsFixClass* target);
	private:
		gpsFixClass* _target;
};

class waypointRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (waypointClass* target);
	private:
		waypointClass* _target;
};

class navRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (navClass* target);
	private:
		navClass* _target;
};

class arduinoStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		bool setTarget (arduinoStateClass* target);
	private:
		arduinoStateClass* _target;
};

class resetArduinoRest : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

class arduinoRESTClass : public RESTdispatchClass {
	public:
		json_t* root (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
		json_t*	defaultFunc (char** tokens, uint32_t* tokenHashes, size_t* tokenLengths, int tokenCnt, int currentToken, char* query, char* method, char* body, int bodyLen);
};

#endif 
