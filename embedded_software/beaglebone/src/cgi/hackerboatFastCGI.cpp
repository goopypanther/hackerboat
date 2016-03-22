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
#include "MurmurHash3.h"
#include "logs.hpp"

RESTdispatchClass *initRESTDispatch (void);

int main (void) {
	char 		uri[LOCAL_BUF_LEN]				= {0}; 
	char 		method[LOCAL_BUF_LEN]			= {0};
	char		query[LOCAL_BUF_LEN]			= {0};
	char		contentLength[LOCAL_BUF_LEN]	= {0}; 
	char		response[LOCAL_BUF_LEN]			= {0};
	char		*tokens[MAX_URI_TOKENS], *body, *tokenState;
	uint32_t	tokenHashes[MAX_URI_TOKENS];
	size_t		tokenLengths[MAX_URI_TOKENS];
	json_t		*responseJSON, *jsonFinal;
	int32_t		uriLen, methodLen, queryLen, bodyLen, contentLenLen;
	int32_t		result, tokenCnt = 0;
	rsize_t		tokenRemain;
	RESTdispatchClass*	root;
	logREST*	log = logREST::instance();
	logError*	err = logError::instance();
	
	// initialize the dispatch hierarchy
	root = initRESTDispatch();
	
	while (FCGI_Accept() >= 0) {
		// read in the critical environment variables and go to next iteration if any fail
		if (getenv_s(&uriLen, uri, LOCAL_BUF_LEN, "REQUEST_URI")) continue;
		if (getenv_s(&methodLen, method, LOCAL_BUF_LEN, "REQUEST_METHOD")) continue;
		if (getenv_s(&queryLen, query, LOCAL_BUF_LEN, "QUERY_STRING")) continue;
		if (getenv_s(&contentLenLen, contentLength, LOCAL_BUF_LEN, "CONTENT_LENGTH")) continue;

		// if there is post data, read it in
		bodyLen 	= atoi(contentLength);
		if (bodyLen > 0) {
			body = calloc(bodyLen, sizeof(char));
			if (body == NULL) continue;
		}
		FCGI_fread(body, 1, bodyLen, FCGI_stdin);
		
		// print the first line of the response
		FCGI_printf("Content-type: application/json\r\n\r\n");
		
		// tokenize the URI & hash the elements
		for (tokenCnt = 0; tokenCnt < MAX_URI_TOKENS; tokenCnt++) {
			if (tokenCnt) {
				tokens[tokenCnt] = strtok_s(NULL, &tokenRemain, "/\\", &tokenState);
			} else {
				tokens[tokenCnt] = strtok_s(uri, &tokenRemain, "/\\", &tokenState);
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
	static gpsFixClass				gpsStruct(GPS_DB_FILE, strlen(GPS_DB_FILE));
	static navClass					navStruct(NAV_DB_FILE, strlen(NAV_DB_FILE));
	static waypointClass			waypointStruct(WP_DB_FILE, strlen(WP_DB_FILE));
	static boneStateClass			boneStruct(BONE_LOG_DB_FILE, strlen(BONE_LOG_DB_FILE));
	static arduinoStateClass		ardStruct(ARD_LOG_DB_FILE, strlen(ARD_LOG_DB_FILE));
	
	// leaf nodes
	static resetArduinoRest			reset("resetArduino");
	static arduinoRESTClass			arduino("a");
	static allDispatchClass			boneAll(&boneStruct);
	static allDispatchClass			gpsAll(&gpsStruct);
	static allDispatchClass			waypointAll(&waypointStruct);
	static allDispatchClass			navAll(&navStruct);
	static allDispatchClass			arduinoAll(&ardStruct);
	static numberDispatchClass		boneNum(&boneStruct);
	static numberDispatchClass		gpsNum(&gpsStruct);
	static numberDispatchClass		waypointNum(&waypointStruct);
	static numberDispatchClass		navNum(&navStruct); 
	static numberDispatchClass		arduinoNum(&ardStruct);
	static countDispatchClass		boneCount(&boneStruct);
	static countDispatchClass		gpsCount(&gpsStruct); 
	static countDispatchClass		waypointCount(&waypointStruct);
	static countDispatchClass		navCount(&navStruct); 
	static countDispatchClass		arduinoCount(&ardStruct);
	static appendDispatchClass		waypointAppend(&waypointStruct);
	static insertDispatchClass		waypointInsert(&waypointStruct);
	
	// root & branch nodes
	static boneStateRESTClass 		bone("boneState", {&boneAll, &boneCount}, 2);
	static gpsRESTClass 			gps("gps", {&gpsAll, &gpsCount}, 2);
	static waypointRESTClass		waypoint("waypoint", {&waypointAll, &waypointCount, &waypointAppend, &waypointInsert}, 4);
	static navRESTClass				nav("nav", {&navAll, &navCount}, 2);
	static arduinoStateRESTClass	ardState("arduinoState", {&arduinoAll, &arduinoCount}, 2);
	static rootRESTClass			root("", {&bone, &gps, &waypoint, &nav, &ardState, &arduino, &reset}, 7);

	bone.addNumber(&boneNum);
	gps.addNumber(&gpsNum);
	waypoint.addNumber(&waypointNum);
	nav.addNumber(&navNum);
	ardState.addNumber(&arduinoNum);
	
	return &root;
}
