/******************************************************************************
 * Hackerboat URL fetch module
 * hal/fetchULR.hpp
 * This module is an interface to the URL fetching functionality.
 * On a setup with a regular network connection, this works through libcurl
 * Use of a FONA will be different
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef FETCHURL_H
#define FETCHURL_H
 
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
#include "hal/inputThread.hpp"

typedef map<std::string, HackerboatState&> objectMap

class Fetch : public InputThread {
	public:
		Fetch (std::string url, objectMap &output);
		bool begin ();								/**< initialize communications (only called once.) */
		bool execute ();							/**< transmit the contents of output and get input */
		bool setUrl (std::string url);				/**< Set the target URL */
		std::string getURL ();						/**< Get the current target URL */
		bool scrub ();								/**< Delete all saved input data */
		objectMap& getInputs ();					/**< Get a reference to the input data map */
		
	private:
		bool globalInit ();					/**< Calls libcurl's global initializer only once */
		size_t uploadCallback (char *bufptr, size_t size, size_t nitems, void *userp);
		size_t downloadCallback (void *buffer, size_t size, size_t nmemb, void *userp);
		static bool initialized = false;	/**< Controls the called of the global init, so it can only be called once */
		objectMap& outputData;
		objectMap* inputData;
};

#endif /* FETCHURL_H */