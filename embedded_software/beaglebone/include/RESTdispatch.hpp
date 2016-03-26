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
#include <vector>
#include <map>

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
		RESTdispatchClass (void) = default;
		/**
		 * @brief Constructor for a dispatch object with given dispatch table
		 */
		RESTdispatchClass (std::map<string,RESTdispatchClass*>& table) : _dispatchTable(table) {};;	/**< Map of leaf nodes to dispatch on */
									
		virtual bool addEntry (RESTdispatchClass *entry);		/**< Add an entry to the dispatch table */
		virtual bool addNumber (RESTdispatchClass *entry);		/**< Add an entry to call when the next token is a decimal number */

		/**
		 * @brief Dispatch on the token designated by currentToken
		 */
		virtual json_t* dispatch (std::vector<std::string> tokens,	/**< Array of tokens from the URI request */
									int currentToken, 				/**< Token to dispatch on */
									std::string query, 				/**< Query string, if any, from the request */
									std::string method, 			/**< Request method, generally either GET or POST */
									std::string body); 				/**< POST request body, if any */
		/**
		 * @brief If there are no more tokens, dispatch() calls this method. Default implementation returns NULL.
		 */							
		virtual json_t* root 	(std::vector<std::string> tokens, 	/**< Array of tokens from the URI request */
									int currentToken, 				/**< Token to dispatch on */
									std::string query, 				/**< Query string, if any, from the request */
									std::string method, 			/**< Request method, generally either GET or POST */
									std::string body)				/**< POST request body, if any */
									{return NULL;};		
		/**
		 * @brief If there are no more tokens, dispatch() calls this method. Default implementation returns NULL.
		 */														
		virtual json_t*	defaultFunc (std::vector<std::string> tokens,/**< Array of tokens from the URI request */
									int currentToken,  				/**< Token to dispatch on */
									std::string query, 				/**< Query string, if any, from the request */
									std::string method, 			/**< Request method, generally either GET or POST */
									std::string body) 				/**< POST request body, if any */
									{return NULL;}; 
		
	protected:	
		RESTdispatchClass* _numberDispatch	= NULL;					/**< Object to dispatch to if the next token is a number */
		std::map<std::string,RESTdispatchClass*>& _dispatchTable;	/**< Dispatch table */
};

/** 
 * @class allDispatchClass
 *
 * @brief This class is for nodes where the last token is 'all'. It is strictly a leaf node.
 *
 */
 
class allDispatchClass : public RESTdispatchClass {
	public:
		allDispatchClass(hackerboatStateClassStorable* target) : _target(target);		/**< Create an 'all' dispatch item attached to hackerboatStateClassStorable target*/
		bool setTarget (hackerboatStateClassStorable* target);		/**< Set the target hackerboatStateClassStorable */
		bool addEntry (RESTdispatchClass *entry) {return false;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		
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
		numberDispatchClass(hackerboatStateClassStorable* target) : _target(target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		
	private:
		hackerboatStateClassStorable* _target;
};

class countDispatchClass : public RESTdispatchClass {
	public:
		countDispatchClass(hackerboatStateClassStorable* target) : _target(target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
	private:
		hackerboatStateClassStorable* _target;
};

class insertDispatchClass : public RESTdispatchClass {
	public:
		insertDispatchClass(hackerboatStateClassStorable* target) : _target(target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
	private:
		hackerboatStateClassStorable* _target;
};

class appendDispatchClass : public RESTdispatchClass {
	public:
		appendDispatchClass(hackerboatStateClassStorable* target) : _target(target);
		bool setTarget (hackerboatStateClassStorable* target);
		bool addEntry (RESTdispatchClass *entry) {return false;};
		bool addNumber (RESTdispatchClass *entry) {return false;};
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
	private:
		hackerboatStateClassStorable* _target;
};

class rootRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		json_t*	defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
};

class boneStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		bool setTarget (boneStateClass* target);
		json_t*	defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
											
	private:
		json_t*	command (std::string body);
		json_t*	waypointNext (std::string body);
		json_t*	waypointStrength (std::string body);
		json_t*	waypointStrengthMax (std::string body);
		json_t*	waypointAccuracy (std::string body);
		json_t*	autonomous (std::string body);
		boneStateClass* _target = NULL;
};

class gpsRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		bool setTarget (gpsFixClass* target);
	private:
		gpsFixClass* _target;
};

class waypointRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		bool setTarget (waypointClass* target);
	private:
		waypointClass* _target;
};

class navRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		bool setTarget (navClass* target);
	private:
		navClass* _target;
};

class arduinoStateRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		bool setTarget (arduinoStateClass* target);
	private:
		arduinoStateClass* _target;
};

class resetArduinoRest : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
};

class arduinoRESTClass : public RESTdispatchClass {
	public:
		json_t* root (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
		json_t*	defaultFunc (std::vector<std::string> tokens, int currentToken, std::string query, std::string method, std::string body);
};

#endif 
