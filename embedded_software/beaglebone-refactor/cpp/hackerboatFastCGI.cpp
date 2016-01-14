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
#include "stateStructTypes.h"
#include "config.h"