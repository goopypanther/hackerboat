/******************************************************************************
 * Hackerboat URL fetch module
 * hal/fetchURL.hpp
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
#include "boatState.hpp"

typedef map<std::string, HackerboatState*> objectMap;
typedef vector<Command> cmdVector;

class Fetch : public InputThread {
	public:
		Fetch (std::string url, 		/**< Create a fetch object pointed at the given URL with the given output objectMap of data to send and input objectMap of data to receive */
				BoatState &state,
				objectMap &output,		
				cmdVector &input);	
		bool begin ();					/**< initialize communications (only called once.) */
		bool execute ();				/**< transmit the contents of output and get input */
		bool setUrl (std::string url);	/**< Set the target URL */
		std::string getURL ();			/**< Get the current target URL */
		bool scrub ();					/**< Delete all saved input data */
		objectMap& getInputs ();		/**< Get a reference to the map of data received */
		bool isComplete();	
		
	private:
		bool globalInit ();					/**< Calls libcurl's global initializer only once */
		size_t uploadCallback (char *bufptr, size_t size, size_t nitems, void *userp);	/**< callback for writing data to remote host */
		size_t downloadCallback (void *buffer, size_t size, size_t nmemb, void *userp);	/**< callback for reading data from remote host */
		static bool global_initialized;		/**< Controls the called of the global init, so it can only be called once */
		bool instance_initialized;			/**< Controls initialization of this instance */
		objectMap& outputData;
		objectMap& inputData;
};

#endif /* FETCHURL_H */