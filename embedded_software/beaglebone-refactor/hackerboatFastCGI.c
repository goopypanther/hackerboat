/******************************************************************************
 * Hackerboat Beaglebone FastCGI module
 * hackerboatFastCGI.c
 * This module handles incoming REST requests
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
#include <dlcfcn.h>
#include "logs.h"
#include "dbReadWrite.h"
#include "config.h"

typedef struct funcDispatch {
	char[30]	name;
	int(*func)(char *, char *, char *, char *, int, int)
} funcDispatch;

// Function declarations
int	restDispatch		(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);
int	getArduinoREST		(char[][MAX_TOKEN_LEN] tokenArray, int tokenCount, 
							char *query, char *response, int buflen);
int boneStateDispatch	(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);
int gpsDispatch			(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);
int waypointDispatch	(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);
int navDispatch			(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);
int arduinoStateDispatch(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen);

// Main function
int main (void) {
	char 		*uri, *method, *query, *body, *token;
	char 		arduinoResponse[ARDUINO_BUF_LEN];
	char 		localResponse[LOCAL_BUF_LEN];
	int 		result, bodyLen, tokenCnt = 0;
	char		tokensArray[MAX_TOKENS][MAX_TOKEN_LEN];
	uint32_t	tokensHashArray[MAX_TOKENS], methodHash;
	int 		tokenCount = 0;
	
	// Function dispatch handle
	void *funcHandle = NULL;
	
	// initialize a handler to call functions by name
	funcHandle = dlopen(NULL, RTLD_LAZY);
	
	while (FCGI_Accept() >= 0) {
		FCGI_printf("Content-type: application/json\r\n\r\n");
		uri 	= getenv("REQUEST_URI");
		method 	= getenv("REQUEST_METHOD");
		query 	= getenv("QUERY_STRING");
		
		// if there is post data, read it in
		bodyLen 	= atoi(getenv("CONTENT_LENGTH"));
		body	 	= malloc(bodyLen);
		FCGI_fread(body, 1, bodyLen, FCGI_stdin);
		
		// tokenize the uri
		if (uri != NULL) {
			bool flag = true;
			while (flag) {
				token = strtok(request, "/\\");
				if (token) {
					snprintf(tokensArray[tokenCount], MAX_TOKEN_LEN, "%s", token);
					tokensHashArray[tokenCount] = murmur3_32(token, strlen(token), HASHSEED);
					tokenCount++;
					if (tokenCount >= MAX_TOKENS) flag = false;
				} else flag = false;
			}
		}
		
		// hash the request method
		methodHash = murmur3_32(method, strlen(method), HASHSEED);
		
		// if there's anything there, dispatch on it. 
		if (tokenCount > 0) {
			if (!strncmp(tokensArray[0], "a", 2)) {								// check if the request begins with /a, which indicates it should go to the Arduino
				getArduinoREST(tokensArray, tokenCount, query, arduinoResponse, ARDUINO_BUF_LEN);	// strip the leading /a and query the Arduino...
				logArduinoREST(tokensArray, tokenCount, query, arduinoResponse);
				FCGI_printf("%s", arduinoResponse);	
			} else {
				result = restDispatch(tokensArray, tokensHashArray, tokenCount, 
					query, body, methodHash, localResponse, bodyLen, LOCAL_BUF_LEN);
				logREST(tokensArray, tokenCount, query, body, bodyLen, method, localResponse);
				FCGI_printf("{\"response\":%s,\"result\":%d,\"id\":%d,\"name\":%s,\"connected\":true}\r\n", 
					localResponse, result, REST_ID, REST_NAME);	
			}
		} 
		
		// clean up section
		free(uri);
		free(method);
		free(query);
		free(body);
		memset(localResponse, 0, LOCAL_BUF_LEN);
		memset(arduinoResponse, 0, ARDUINO_BUF_LEN);
		memset(tokensArray, 0, MAX_TOKENS * MAX_TOKEN_LEN);
		memset(tokensHashArray, 0, MAX_TOKENS * sizeof(uint32_t));
		tokenCount = 0;
		methodHash = 0;
		FCGI_Finish();
	}
	return 0;
}

// Dispatch functions
int	restDispatch (char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
					int tokenCount, char *query, char *body, uint32_t method, 
					char *response, int bodyLen, int responseLen) {
	switch (tokenHashArray[0]) {
		case (murmur3_32("boneState", 9, HASHSEED)):
			return boneStateDispatch(tokenArray, tokenHashArray, tokenCount, 
									query, body, method, response, bodyLen,
									responseLen);
		case (murmur3_32("gps", 3, HASHSEED)):
			return gpsDispatch(tokenArray, tokenHashArray, tokenCount, 
									query, body, method, response, bodyLen,
									responseLen);
		case (murmur3_32("waypoint", 8, HASHSEED)):
			return waypointDispatch(tokenArray, tokenHashArray, tokenCount, 
									query, body, method, response, bodyLen,
									responseLen);
		case (murmur3_32("nav", 2, HASHSEED)):
			return navDispatch(tokenArray, tokenHashArray, tokenCount, 
									query, body, method, response, bodyLen,
									responseLen);
		case (murmur3_32("arduinoState", 12, HASHSEED)):
			return arduinoStateDispatch(tokenArray, tokenHashArray, tokenCount, 
									query, body, method, response, bodyLen,
									responseLen);
		default:
			return -1;
	}
}

int boneStateDispatch (char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
					int tokenCount, char *query, char *body, uint32_t method, 
					char *response, int bodyLen, int responseLen) {
	bool 			write = false;
	int 			result = 0;
	long 			select;
	boneState		vec;
	boneState		*logs;
	json_t			*resp, *inp;	
	json_error_t	*err;
	char			*buf;
	int				records, count, input;
	double			inputFloat;
	long			start;
	timespec		thisTime;
	
	// get the current time
	clock_gettime(CLOCK_REALTIME, &thisTime);
	
	// check whether we're going to write or not.
	if (method == murmur3_32("POST", 4, HASHSEED)) {
		write = true;
	}
	
	// grab the current state vector & modify timestamp to current time (if we're writing)
	if (!getBoneVector(&vec, -1, 1)) result = -1;
	if (write) vec.uTime = ((thisTime.tv_sec * 1000000) + (long)(thisTime.tv_nsec/1000));

	// figure out what to do with this...
	if (tokenCount == 1) {
		if (!result) resp = packBoneVector(vec);
	} else if (sscanf(tokenArray[1], "%dl", select)) {
		if (!getBoneVector(&vec, select, 1)) result = -1;
		if (!result) resp = packBoneVector(vec);
	} else if (tokenHashArray[1] == murmur3_32("all", 3, HASHSEED)) {
		records = countBoneVector();
		logs = malloc(records*sizeof(boneState));
		if (getBoneVector(logs, start, records) {
			resp = json_array();
			for (int i; i < records; i++) {
				json_array_append(resp, packBoneVector(logs[i]));
			}
		} else result = -1;
	} else if (tokenHashArray[1] == murmur3_32("count", 5, HASHSEED)) {
		resp = json_pack("{s:i}", "boneStateCount", countBoneVector);
	} else if (tokenHashArray[1] == murmur3_32("command", 7, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:i}", "command", &input);
			if ((!result) && (input >= 0) && (input < boneStateCount)) {
				vec.state = input;
				snprintf(vec.stateString, STATE_STRING_LEN, "%s", boneStates[input]);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:i,s:i}", "success", 1, "command", 
								vec.command, "commandString", vec.commandString);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:i,s:i}", "command", vec.command,
								"commandString", vec.commandString);
		}
	} else if (tokenHashArray[1] == murmur3_32("waypointNext", 12, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:i}", "waypointNext", &input);
			if ((!result) && (input >= 0) && (input < countWaypoints())) {
				vec.waypointNext = input;
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:i}", "success", 1, 
								"waypointNext", vec.waypointNext);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:i}", "waypointNext", vec.waypointNext);
		}
	} else if (tokenHashArray[1] == murmur3_32("waypointStrength", 16, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:f}", "waypointStrength", &inputFloat);
			if ((!result) && (inputFloat >= 0) && (inputFloat < 200.0)) {
				vec.waypointStrength = inputFloat;
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:f}", "success", 1,
								"waypointStrength", vec.waypointStrength);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:f}", "waypointStrength", vec.waypointStrength);
		}
	} else if (tokenHashArray[1] == murmur3_32("waypointAccuracy", 16, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:f}", "waypointAccuracy", &inputFloat);
			if ((!result) && (inputFloat >= 0) && (inputFloat < 100.0)) {
				vec.waypointAccuracy = inputFloat;
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:f}", "success", 1,
								"waypointAccuracy", vec.waypointAccuracy);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:f}", "waypointAccuracy", vec.waypointAccuracy);
		}
	} else if (tokenHashArray[1] == murmur3_32("waypointStrengthMax", 19, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:f}", "waypointStrenghtMax", &inputFloat);
			if ((!result) && (inputFloat >= 0) && (inputFloat < 200.0)) {
				vec.waypointStrenghtMax = inputFloat;
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:f}", "success", 1,
				"waypointStrengthMax", vec.waypointStrengthMax);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:f}", "waypointStrengthMax", vec.waypointStrengthMax);
		}
	} else if (tokenHashArray[1] == murmur3_32("offshore", 8, HASHSEED)) {
		if (write && !result) {
			inp = json_loads(body, 0, err);
			result = json_unpack(inp, "{s:b}", "offshore", &input);
			if (!result) {
				if (input) {
					vec.offshore = true;
				} else {
					vec.offshore = false;
				}
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
			if ((!result) && (writeBoneVector(&vec))) {
				resp = json_pack("{s:b,s:b}", "success", 1,
								"offshore", vec.offshore);
			} else {
				resp = json_pack("{s:b}", "success", 0);
				result = -1;
			}
		} else {
			resp = json_pack("{s:b}", "offshore", vec.offshore);
		}
	} else result = -1;
	
	// pack up the response and send it along
	if (resp) {
		buf = json_dumps(resp, JSON_COMPACT);
		snprintf(response, buflen, "%s", buf);
	}
	
	free(inp);
	free(err);
	free(resp);
	free(buf);
	free(logs);
	return result;
}

int gpsDispatch	(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
					int tokenCount, char *query, char *body, uint32_t method, 
					char *response, int bodyLen, int responseLen) {
	char 			*token;
	bool 			write = false;
	int 			result = 0;
	gpsVector		vec;
	gpsVector		*logs;
	json_t			*resp;
	char			*buf;
	
	if (!getGPSvector(&vec, -1, 1)) result = -1;
	
	// figure out what to do with this...
	if (tokenCount == 1) {
		if (!result) resp = packGPSVector(vec);
	} else if (sscanf(tokenArray[1], "%dl", select)) {
		if (!getGPSVector(&vec, select, 1)) result = -1;
		if (!result) resp = packGPSVector(vec);
	} else if (tokenHashArray[1] == murmur3_32("all", 3, HASHSEED)) {
		records = countGPSVector();
		logs = malloc(records*sizeof(gpsVector));
		if (getGPSVector(logs, start, records) {
			resp = json_array();
			for (int i; i < records; i++) {
				json_array_append(resp, packGPSVector(logs[i]));
			}
		} else result = -1;
	} else result = -1;
	
	// pack up the response and send it along
	if (resp) {
		buf = json_dumps(resp, JSON_COMPACT);
		snprintf(response, buflen, "%s", buf);
	}
	
	free(resp);
	free(buf);
	free(logs);
	return result;
}

int waypointDispatch (char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
					int tokenCount, char *query, char *body, uint32_t method, 
					char *response, int bodyLen, int responseLen) {
	char 			*token;
	bool 			write = false;
	int 			result = 0;
	json_t			*resp, *inp;	
	json_error_t	*err;
	char			*buf;
	boneState		vec;
	waypointStruct	waypoint;
	waypointStruct	*logs;
	int 			index
	
	
	
	// pack up the response and send it along
	if (resp) {
		buf = json_dumps(resp, JSON_COMPACT);
		snprintf(response, buflen, "%s", buf);
	}
	
	free(inp);
	free(err);
	free(resp);
	free(buf);
	free(logs);
	return result;
}

int navDispatch	(char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
					int tokenCount, char *query, char *body, uint32_t method, 
					char *response, int bodyLen, int responseLen) {

}

int arduinoStateDispatch (char[][MAX_TOKEN_LEN] tokenArray, uint32_t *tokenHashArray, 
							int tokenCount, char *query, char *body, uint32_t method, 
							char *response, int bodyLen, int responseLen) {
					
}

int	getArduinoREST (char[][MAX_TOKEN_LEN] tokenArray, int tokenCount, 
							char *query, char *response, int buflen) {
}
