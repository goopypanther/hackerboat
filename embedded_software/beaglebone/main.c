/**
 * @file main.c
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version
 * @since Jun 29, 2015
 */


#include "includes.h"

// Defines


// Function prototypes


// Static variables

int main (int argc, char* argv[]) {
	int returnState;

	argsParseInputParams(argc, argv);

	logOpen(argsReturnLogFile());
	udpOpenSocket(argsReturnTargetIp());

	returnState = 0;

	return (returnState);
}
