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
#include <MurmurHash3.h>
#include "logs.hpp"

RESTdispatchClass *initRESTDispatch (void);

int main (void) {
	char 		*uri, *method, *query, *contentLength;
	char		response[LOCAL_BUF_LEN]			= {0};
	char		*tokens[MAX_URI_TOKENS], *body, *tokenState;
	uint32_t	tokenHashes[MAX_URI_TOKENS];
	size_t		tokenLengths[MAX_URI_TOKENS];
	json_t		*responseJSON, *jsonFinal;
	int32_t		uriLen, methodLen, queryLen, bodyLen, contentLenLen;
	int32_t		result, tokenCnt = 0;
	size_t		tokenRemain;
	RESTdispatchClass*	root;
	logREST*	log = logREST::instance();
	logError*	err = logError::instance();
	
	// initialize the dispatch hierarchy
	root = initRESTDispatch();
	
	while (FCGI_Accept() >= 0) {
		// read in the critical environment variables and go to next iteration if any fail
		if (!(uri = getenv("REQUEST_URI"))) continue;
		if (!(method = getenv("REQUEST_METHOD"))) continue;
		if (!(query = getenv("QUERY_STRING"))) continue;
		if (!(contentLength = getenv("CONTENT_LENGTH"))) continue;

		// figure out the lengths of the various strings
		uriLen = strlen(uri);
		methodLen = strlen(method);
		queryLen = strlen(query);
		contentLenLen = strlen(contentLength);
		
		// if there is post data, read it in
		bodyLen 	= atoi(contentLength);
		if (bodyLen > 0) {
			body = (char *)calloc(bodyLen, sizeof(char));
			if (body == NULL) continue;
		}
		FCGI_fread(body, 1, bodyLen, FCGI_stdin);
		
		// print the first line of the response
		FCGI_printf("Content-type: application/json\r\n\r\n");
		
		// tokenize the URI & hash the elements
		for (tokenCnt = 0; tokenCnt < MAX_URI_TOKENS; tokenCnt++) {
			if (tokenCnt) {
				tokens[tokenCnt] = strtok(NULL, "/\\");
			} else {
				tokens[tokenCnt] = strtok(uri, "/\\");
			}
			if (tokens[tokenCnt] == NULL) break;
			tokenLengths[tokenCnt] = strlen(tokens[tokenCnt]);
			MurmurHash3_x86_32(&(tokens[tokenCnt]), tokenLengths[tokenCnt], 
								HASHSEED, &(tokenHashes[tokenCnt]));
		}
		
		// dispatch on the URI
		responseJSON = root->dispatch(tokens, tokenHashes, tokenLengths, tokenCnt, 0, query, method, body, bodyLen);
		
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
		log->write(tokens, tokenCnt, query, body, bodyLen, method, response);
		log->close();
		
		// clean up
		free(body);
		json_decref(jsonFinal);
		for (uint8_t i = 0; i < MAX_URI_TOKENS; i++) {
			tokens[i] = NULL;
		}
	}
}

RESTdispatchClass *initRESTDispatch (void) {
	// data structure objects
	gpsFixClass				*gps = new gpsFixClass;
	navClass				*nav = new navClass;
	waypointClass			*waypoint = new waypointClass;
	boneStateClass			*boat = new boneStateClass;
	arduinoStateClass		*arduinoState = new arduinoStateClass;
	
	// leaf nodes
	resetArduinoRest		*reset = new resetArduinoRest((const)std::string("resetArduino"));
	arduinoRESTClass		*arduino = new arduinoRESTClass((const)std::string("a"));
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
	
	// root & branch nodes
	boneStateRESTClass 		*boatREST = new boneStateRESTClass("boneState", {&boneAll, &boneCount}, 2);
	gpsRESTClass 			*gpsREST = new gpsRESTClass("gps", {&gpsAll, &gpsCount}, 2);
	waypointRESTClass		*waypointREST = new waypointRESTClass("waypoint", {&waypointAll, &waypointCount, &waypointAppend, &waypointInsert}, 4);
	navRESTClass			*navREST = new navRESTClass("nav", {&navAll, &navCount}, 2);
	arduinoStateRESTClass	*arduinoREST = new arduinoStateRESTClass("arduinoState", {&arduinoAll, &arduinoCount}, 2);
	rootRESTClass			*root = new rootRESTClass("", {&boatREST, &gpsREST, &waypointREST, &navREST, &arduinoREST, &arduino, &reset}, 7);

	boatREST->addNumber(boneNum);
	gpsREST->addNumber(gpsNum);
	waypointREST->addNumber(waypointNum);
	navREST->addNumber(navNum);
	arduinoREST->addNumber(arduinoNum);
	
	return root;
}
