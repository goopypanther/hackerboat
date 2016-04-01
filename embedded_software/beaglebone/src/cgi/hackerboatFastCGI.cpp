/******************************************************************************
 * Hackerboat Beaglebone FastCGI program
 * hackerboatFastCGI.cpp
 * This program handles incoming REST requests
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Dec 2015
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
#include "stateStructTypes.hpp"
#include "RESTdispatch.hpp"
#include "config.h"
#include "logs.hpp"
#include "boneState.hpp"
#include "arduinoState.hpp"
#include "navigation.hpp"
#include "gps.hpp"

#include <string>
#include <map>
#include <vector>

RESTdispatchClass *initRESTDispatch (void);

int main (void) {
	char 		*ptr, *bodyBuf;
	char		response[LOCAL_BUF_LEN]			= {0};
	json_t		*responseJSON, *jsonFinal;
	size_t 		bodyLen, tokenLen;
	size_t		tokenPos = 0, nextTokenPos = 0;
	RESTdispatchClass*	root;
	logREST*	log = logREST::instance();
	logError*	err = logError::instance();
	
	// initialize the dispatch hierarchy
	root = initRESTDispatch();
	
	while (FCGI_Accept() >= 0) {
		std::string	uri, method, query, body;
		std::vector<std::string> tokens;
		// read in the critical environment variables and go to next iteration if any fail
		// Note that the assignment in if statements is deliberate to detect the null pointer
		if (ptr = getenv("PATH_INFO")) {
			uri.assign(ptr);
		} else continue;
		if (ptr = getenv("REQUEST_METHOD")) {
			method.assign(ptr);
		} else continue;
		if (ptr = getenv("QUERY_STRING")) {
			query.assign(ptr);
		} else continue;
		if (ptr = getenv("CONTENT_LENGTH")) {
		        bodyLen = ::atoi(ptr);
		} else {
	                 // For GET requests, etc., we don't get a CONTENT_LENGTH, which is OK.
		         bodyLen = 0;
		}
		
		// if there is post data, read it in
		if (bodyLen > 0) {
			bodyBuf = (char *)calloc(bodyLen, sizeof(char));
			if (bodyBuf == NULL) continue;
			FCGI_fread(bodyBuf, 1, bodyLen, FCGI_stdin);
			body.assign(bodyBuf, bodyLen);
			free(bodyBuf);
		}

		// print the first line of the response
		FCGI_printf("Content-type: application/json\r\n\r\n");
		
		// chop up the URI and move the elements into a vector of strings
		tokens.clear();
		tokenPos = 0;
		while (tokenPos < uri.length() && uri.at(tokenPos) == '/')
			tokenPos ++;
		nextTokenPos = tokenPos;
		while (tokenPos != std::string::npos) {
			nextTokenPos = uri.find_first_of("/\\", tokenPos+1);
			if (nextTokenPos == std::string::npos) {
			        tokenLen = uri.length() - tokenPos;
			} else {
				tokenLen = (nextTokenPos - tokenPos);
				nextTokenPos ++;
			}
			tokens.push_back(uri.substr(tokenPos, tokenLen));
			tokenPos = nextTokenPos;
		}
		
		// dispatch on the URI
		responseJSON = root->dispatch(tokens, 0, query, method, body);
		
		// add some things to the JSON response...
		jsonFinal = json_pack("{sOsissss}", "response", responseJSON, 
								"id", REST_ID, 
								"name", REST_NAME,
								"connected", "true");
		
		// print the result back to the client
		snprintf(response, LOCAL_BUF_LEN, "%s", json_dumps(jsonFinal, JSON_COMPACT));
		FCGI_printf("%s\r\n", response);

		// log everything
		log->open(REST_LOGFILE);
		log->write(tokens, query, body, method, response);
		log->close();
		
		// clean up
		tokens.clear();
		json_decref(jsonFinal);
		tokenPos = 0;
	}
}

RESTdispatchClass *initRESTDispatch (void) {
	// data structure objects
	gpsFixClass				*gps = new gpsFixClass();
	navClass				*nav = new navClass();
	waypointClass			*waypoint = new waypointClass();
	boneStateClass			*boat = new boneStateClass();
	arduinoStateClass		*arduinoState = new arduinoStateClass();
	
	// leaf nodes
	resetArduinoRest		*reset = new resetArduinoRest();
	arduinoRESTClass		*arduino = new arduinoRESTClass();
	allDispatchClass		*boneAll = new allDispatchClass(boat);
	allDispatchClass		*gpsAll = new allDispatchClass(gps);
	allDispatchClass		*waypointAll = new allDispatchClass(waypoint);
	allDispatchClass		*navAll = new allDispatchClass(nav);
	allDispatchClass		*arduinoAll = new allDispatchClass(arduinoState);
	numberDispatchClass		*boneNum = new numberDispatchClass(boat);
	numberDispatchClass		*gpsNum = new numberDispatchClass(gps);
	numberDispatchClass		*waypointNum = new numberDispatchClass(waypoint);
	numberDispatchClass		*navNum = new numberDispatchClass(nav); 
	numberDispatchClass		*arduinoNum = new numberDispatchClass(arduinoState);
	countDispatchClass		*boneCount = new countDispatchClass(boat);
	countDispatchClass		*gpsCount = new countDispatchClass(gps); 
	countDispatchClass		*waypointCount = new countDispatchClass(waypoint);
	countDispatchClass		*navCount = new countDispatchClass(nav); 
	countDispatchClass		*arduinoCount = new countDispatchClass(arduinoState);
	appendDispatchClass		*waypointAppend = new appendDispatchClass(waypoint);
	insertDispatchClass		*waypointInsert = new insertDispatchClass(waypoint);
	
	// dispatch map declarations and branch nodes
	static map<std::string, RESTdispatchClass&> boatMap = {{"all", *boneAll}, {"count", *boneCount}};
	boneStateRESTClass 	*boatREST = new boneStateRESTClass(&boatMap);
	static map<std::string, RESTdispatchClass&> gpsMap = {{"all", *gpsAll}, {"count", *gpsCount}};
	gpsRESTClass *gpsREST = new gpsRESTClass(&gpsMap);
	static map<std::string, RESTdispatchClass&> wpMap = {{"all", *waypointAll}, {"count", *waypointCount}, 
													{"append", *waypointAppend}, {"insert", *waypointInsert}};
	waypointRESTClass *waypointREST = new waypointRESTClass(&wpMap);
	static map<std::string, RESTdispatchClass&> navMap = {{"all", *navAll}, {"count", *navCount}};														
	navRESTClass *navREST = new navRESTClass(&navMap);
	static map<std::string, RESTdispatchClass&> ardMap = {{"all", *arduinoAll}, {"count", *arduinoCount}};
	arduinoStateRESTClass *arduinoREST = new arduinoStateRESTClass(&ardMap);	
	static map<std::string, RESTdispatchClass&> rootMap = {{"boneState", *boatREST}, {"gps", *gpsREST}, 
													{"waypoint", *waypointREST}, {"nav", *navREST}, 
													{"arduinoState", *arduinoREST}, {"arduino", *arduino}, 
													{"resetArduino", *reset}};							;														
	rootRESTClass *root = new rootRESTClass(&rootMap);
					
	// number assignments
	boatREST->addNumber(boneNum);
	gpsREST->addNumber(gpsNum);
	waypointREST->addNumber(waypointNum);
	navREST->addNumber(navNum);
	arduinoREST->addNumber(arduinoNum);
	
	// set targets
	boatREST->setTarget(boat);
	gpsREST->setTarget(gps);
	waypointREST->setTarget(waypoint);
	navREST->setTarget(nav);
	arduinoREST->setTarget(arduinoState);
	
	return root;
}
