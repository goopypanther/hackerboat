/******************************************************************************
 * Hackerboat Beaglebone logging module
 * logs.h
 * This modules is compiled into the other modules to give a common interface
 * for logging
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Dec 2015
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include "config.h" 
 
int logArduinoREST(char[][MAX_TOKEN_LEN] tokenArray, int tokenCount, char *query, char *response);
int logREST(char[][MAX_TOKEN_LEN] tokenArray, int tokenCount, char *query, char *body, int bodyLen, char *method, char *response);