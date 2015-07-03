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
	char beginMsg[] = "Beginning test...";
	mavlink_message_t packet;

	argsParseInputParams(argc, argv);

	logOpen(argsReturnLogFile());
	udpOpenSocket(argsReturnTargetIp());

	logLine(beginMsg);

	mavlink_msg_scaled_imu_pack(1,
								MAV_COMP_ID_SYSTEM_CONTROL,
								&packet,
								0,
								1,
								1,
								1,
								1,
								1,
								1,
								1,
								1,
								1);

	logPacket(&packet);

	udpCloseSocket();
	logClose();

	returnState = 0;

	return (returnState);
}
