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
void mainWaitSliceTask(void);

// Static variables

int main (int argc, char* argv[]) {
	int returnState;
	nmea_time_t curTime;

	//argsParseInputParams(argc, argv);
	//logOpen(argsReturnLogFile());
	//udpOpenSocket(argsReturnTargetIp());
	//uartInit(argsReturnGpsSerialDevice(), argsReturnLowLevelSerial());

	//while (Neo6mGetStatus() == FALSE) {}

	printf("%llu\n", currentTimeMsSinceEpoch());
	mainWaitSliceTask();
	printf("%llu\n", currentTimeMsSinceEpoch());
	mainWaitSliceTask();
	printf("%llu\n", currentTimeMsSinceEpoch());

	//logClose();

	returnState = 0;
	return (returnState);
}

/**
 * Waits for next slice, every 10ms
 */
void mainWaitSliceTask(void) {
	static uint64_t lastTime;
	static uint32_t isInit = TRUE;

	if (isInit == TRUE) {
		isInit = FALSE;

		lastTime = currentTimeMsSinceEpoch();

	} else {
		while ((currentTimeMsSinceEpoch() - lastTime) < 10) {}

		lastTime = currentTimeMsSinceEpoch();
	}
}
