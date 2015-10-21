/**
 * @file heartbeat.c
 * @brief tracks heartbeat packets
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 18, 2015
 */


#include "includes.h"

// Defines
#define PANIC_TIME 10

// Function prototypes
void heartbeatReceive(const mavlink_message_t *msg);
void heartbeatSend(void);
uint32_t heartbeatCheckPanicHost(void);
uint32_t heartbeatCheckPanicLowLevel(void);

// Static variables
static uint64_t lastHostHeartbeat;
static uint64_t lastLowLevelHeartbeat;

/**
 * Receive heartbeat packets and feed watchdog
 *
 * @param msg mavlink heartbeat message
 */
void heartbeatReceive(const mavlink_message_t *msg) {
	// Check if heartbeat signal received
	if (msg->msgid == MAVLINK_MSG_ID_HEARTBEAT) {
		// Check if heartbeat from host
		if (msg->sysid == boatStateReturnShoreSystemId()) {
			lastHostHeartbeat = currentTimeUsSinceEpoch();

		} else if (msg->sysid == boatStateReturnLowLevelSystemId()) {
		// Check if heartbeat from lowlevel
			lastLowLevelHeartbeat = currentTimeUsSinceEpoch();

		} else {}

	} else {}
}

/**
 * Send heartbeat packet
 */
void heartbeatSend(void) {
	mavlink_message_t heartbeatMsg;

	mavlink_msg_heartbeat_pack(boatStateReturnSystemId(),
							   MAV_COMP_ID_SYSTEM_CONTROL,
							   &heartbeatMsg,
							   boatStateReturnVehicleType(),
							   boatStateReturnAutopilotType(),
							   boatStateReturnMode(),
							   boatStateReturnCustomMode(),
							   boatStateReturnState());

	mavlinkWrapperSend(&heartbeatMsg);
}

/**
 * Check if time since last heartbeat exceeds \c PANIC_TIME
 *
 * @return true if time exceeds \c PANIC_TIME
 */
uint32_t heartbeatCheckPanicHost(void) {
	uint32_t panic;
	uint64_t timeElapsed;

	timeElapsed = currentTimeUsSinceEpoch() - lastHostHeartbeat;

	if (timeElapsed > (PANIC_TIME * 1000000)) {
		panic = TRUE;
	} else {
		panic = FALSE;
	}

	return (panic);
}

/**
 * Check if time since last heartbeat exceeds \c PANIC_TIME
 *
 * @return true if time exceeds \c PANIC_TIME
 */
uint32_t heartbeatCheckPanicLowLevel(void) {
	uint32_t panic;
	uint64_t timeElapsed;

	timeElapsed = currentTimeUsSinceEpoch() - lastLowLevelHeartbeat;

	if (timeElapsed > (PANIC_TIME * 1000000)) {
		panic = TRUE;
	} else {
		panic = FALSE;
	}

	return (panic);
}
