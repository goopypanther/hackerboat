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
#define MS_PER_TIMESLICE 10
#define ONCE_PER_SECOND 100
#define ONCE_PER_FIVE_SECONDS 500

// Function prototypes
void mainWaitSliceTask(void);
void mainParseIncomingPacketTask(void);
void mainStateSelectTask(void);
void mainHeartbeatTask(void);
void mainSystemStatusTask(void);
void mainGpsTask(void);

// Static variables

int main (int argc, char* argv[]) {
	argsParseInputParams(argc, argv);

	logOpen(argsReturnLogFile());

	ioConfigInit();

	udpOpenSocket(argsReturnTargetIp());
	uartInit(argsReturnGpsSerialDevice(), argsReturnLowLevelSerial());

	// Main loop
	for (;;) {
		mainWaitSliceTask();

		mainParseIncomingPacketTask();
		mainStateSelectTask();

		mainHeartbeatTask();
		mainSystemStatusTask();

		mainGpsTask();

	}

	logClose();

	return (0);
}

/**
 * Waits for next slice, every 10ms
 */
void mainWaitSliceTask(void) {
	static uint64_t nextTime;
	static uint32_t isInit = TRUE;

	if (isInit == TRUE) {
		isInit = FALSE;

		nextTime = currentTimeUsSinceEpoch();

	} else {
		// Block until elapsed time > timeslice period
		while ((currentTimeUsSinceEpoch() - nextTime) < (MS_PER_TIMESLICE * 1000)) {}

		nextTime += (MS_PER_TIMESLICE * 1000); // Move next time into the future
	}
}

/**
 * Parse incoming packets and call appropriate packet handling function
 */
void mainParseIncomingPacketTask(void) {
	mavlink_message_t incomingMsg;
	uint32_t newPacketReceived;
	mavlink_manual_control_t controll; // TODO: remove this

	newPacketReceived = mavlinkWrapperReceive(); // Receive new packet

	if (newPacketReceived == TRUE) {
		incomingMsg = mavlinkWrapperReturnMessage();

		switch (incomingMsg.msgid) {
		case MAVLINK_MSG_ID_HEARTBEAT:
		    heartbeatReceive(&incomingMsg);
		    break;

		case MAVLINK_MSG_ID_MISSION_REQUEST_PARTIAL_LIST:
		case MAVLINK_MSG_ID_MISSION_REQUEST:
		case MAVLINK_MSG_ID_MISSION_CURRENT:
		case MAVLINK_MSG_ID_MISSION_ACK:
		case MAVLINK_MSG_ID_MISSION_ITEM:
		case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
		case MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST:
		case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
		case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
		case MAVLINK_MSG_ID_MISSION_COUNT:
		    // TODO: missionReceive(&incomingMsg);
		    break;

		case MAVLINK_MSG_ID_COMMAND_LONG:
		    // TODO: commandReceive(&incomingMsg);
			commandReceive(&incomingMsg);
		    break;

		case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
		case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
		    paramReceive(&incomingMsg);
		    break;

		case MAVLINK_MSG_ID_SET_MODE:
			boatStateReceive(&incomingMsg);
			break;

		//case MAVLINK_MSG_ID_MANUAL_SETPOINT:
			// TODO manualSetpointReceive()
			//break;

		case MAVLINK_MSG_ID_BATTERY_STATUS:
			voltageReceive(&incomingMsg);
			break;

		case MAVLINK_MSG_ID_SYS_STATUS:
			// TODO
			break;

		case MAVLINK_MSG_ID_ATTITUDE:
			// TODO
			break;

		default:
			break;
		}
	} else {}
}

/**
 * Select system state
 */
void mainStateSelectTask(void) {
	switch (boatStateReturnState()) {
	case MAV_STATE_BOOT:
		if (Neo6mGetStatus() == TRUE) {
			boatStateSetState(MAV_STATE_STANDBY);
			logLine("*** Acquired GPS Lock ***");
		} else {}
		break;

	case MAV_STATE_STANDBY:

		break;

	case MAV_STATE_ACTIVE:
		if (heartbeatCheckPanicHost()) {
			boatStateSetState(MAV_STATE_EMERGENCY);
			logLine("*** Emergency, lost comms with groundcontrol ***");
		} else {}

		if (heartbeatCheckPanicLowLevel()) {
			boatStateSetState(MAV_STATE_EMERGENCY);
			logLine("*** Emergency, lost comms with lowlevel ***");
		} else {}
		break;

	case MAV_STATE_CRITICAL:
		break;

	default:
		break;
	}
}

/**
 * Sends heartbeat every second
 */
void mainHeartbeatTask(void) {
	static uint32_t sliceCount = 0;
	mavlink_message_t heartbeatMsg;

	if (sliceCount >= ONCE_PER_SECOND) {
		sliceCount = 0;

		// Task begins
		heartbeatSend();

	} else {
		sliceCount++;
	}
}

/**
 * Sends system status every second
 */
void mainSystemStatusTask(void) {
	static uint32_t sliceCount = 0;
	mavlink_message_t systemStatusMsg;

	if (sliceCount >= ONCE_PER_SECOND) {
		sliceCount = 0;

		// Task begins
		mavlink_msg_sys_status_pack(boatStateReturnSystemId(),
									MAV_COMP_ID_SYSTEM_CONTROL,
									&systemStatusMsg,
									boatStateReturnSensors(),
									boatStateReturnSensors(),
									boatStateReturnSensors(),
									500,
									voltageReturnBatteryVoltage(),
									-1,
									-1,
									0,
									0,
									0,
									0,
									0,
									0);

		mavlinkWrapperSend(&systemStatusMsg);

	} else {
		sliceCount++;
	}
}

/**
 * Sends GPS location of boat every 5 sec
 */
void mainGpsTask(void) {
	static uint32_t sliceCount = 0;
	nmea_rmc_t gpsData;
	mavlink_message_t gpsMsg;

	if (sliceCount >= ONCE_PER_FIVE_SECONDS) {
		sliceCount = 0;

		// Task begins
		gpsData = Neo6mGetData();

		// Check if we have GPS lock
		if (gpsData.status == TRUE) {
			mavlink_msg_global_position_int_pack(boatStateReturnSystemId(),
												 MAV_COMP_ID_SYSTEM_CONTROL,
												 &gpsMsg,
												 ((uint32_t) (currentTimeUsSinceEpoch() / 1000)),
												 gpsData.space.lat,
												 gpsData.space.lon,
												 0,
												 0,
												 ((int16_t) gpsData.velocity.speed),
												 0,
												 0,
												 ((uint16_t) (gpsData.velocity.headingMadeGood * 100)));

			mavlinkWrapperSend(&gpsMsg);

		} else {}
	} else {
		sliceCount++;
	}
}
