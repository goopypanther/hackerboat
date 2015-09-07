/**
 * @file command.c
 * @brief Changes boat state according to received commands
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Sep 5, 2015
 */


#include "includes.h"

// Defines


// Function prototypes
void commandReceive(const mavlink_message_t *msg);

// Static variables

/**
 * Receives and parses command packets from groundcontrol
 *
 * @param msg pointer to received mavlink packet
 */
void commandReceive(const mavlink_message_t *msg) {
	mavlink_command_long_t decodedCommandMessage;

	// Check if command long received
	if (msg->msgid == MAVLINK_MSG_ID_COMMAND_LONG) {
		// Check if command long from host
		if (msg->sysid == boatStateReturnShoreSystemId()) {

			mavlink_msg_command_long_decode(msg, &decodedCommandMessage); // Decode command long

			// Check if command is for us
			if (decodedCommandMessage.target_system == boatStateReturnSystemId()) {
				// Check if takeoff button pressed
				if (decodedCommandMessage.command == MAV_CMD_NAV_TAKEOFF) {
					// Check if we're in an armed mode
					if ((boatStateReturnMode() == MAV_MODE_AUTO_ARMED) || (boatStateReturnMode() == MAV_MODE_MANUAL_ARMED)) {

						boatStateSetState(MAV_STATE_ACTIVE);

					} else {}
				} else if (decodedCommandMessage.command == MAV_CMD_NAV_LAND) {
				// Check if landing button pressed

					boatStateSetState(MAV_STATE_STANDBY);

				} else {}

			} else {}

		} else {}

	} else {}
}
