/******************************************************************************
 * Hackerboat Beaglebone Navigator program
 * hackerboatNavigator.cpp
 * This program handles navigation
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Feb 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>
#include "stateStructTypes.hpp"
#include "config.h"
#include "logs.hpp"
#include "navigation.hpp"
#include "location.hpp"

int *navInf(navigatorBase *nav);

int main (void) {
	navClass nav (NAV_DB_FILE, strlen(NAV_DB_FILE));
	navigatorBase *navInf;
	int navCount;
	
	navCount = initNav(navInf);
	
	for (;;) {
		if (nav.openFile()) {
			if (nav.getRecord(nav.count())) {
				if (navInf) {
					nav.clearVectors();
					for (uint16_t i = 0; i < navCount; i++) {
						nav.appendVector(navInf[i].calc());
					}
				}
				nav.calc();
				nav.writeRecord();
			}
			nav.closeFile();
		}
	}
}

int *navInf(navigatorBase *nav) {
	nav = NULL;
	return 0;
}