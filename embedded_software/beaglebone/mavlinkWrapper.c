/**
 * @file mavlinkWrapper.c
 * @brief Send and receive mavlink packets over UDP interface
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 17, 2015
 */


#include "includes.h"

// Defines
#define BUFFER_LENGTH 2048

// Function prototypes


// Static variables
static mavlink_message_t incomingMessageHost;
static mavlink_status_t incomingMessageStatusHost;

static mavlink_message_t incomingMessageLowLevel;
static mavlink_status_t incomingMessageStatusLowLevel;

/**
 * Sends mavlink packet over UDP to host computer
 *
 * @param packet Mavlink packet to send
 */
void mavlinkWrapperHostSend(mavlink_message_t *packet) {
	char packetBuffer[sizeof(mavlink_message_t)];
	uint32_t packetBufferFilled;

	packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, packet); // Copy packet to buffer

	udpSend(packetBuffer, packetBufferFilled); // Send contents of buffer over UDP

	logPacket(packet); // Log transmission of packet
}

/**
 * Receive and decode mavlink packets from host computer over UDP
 */
void mavlinkWrapperHostReceive() {
	char buffer[BUFFER_LENGTH];
	uint32_t bufferLengthReceived;
	uint32_t i;
	uint8_t messageFound = 0;

	bufferLengthReceived = udpReceive(buffer, sizeof(buffer)); // Receive packet from UDP socket

	// Check if something was received
	if (bufferLengthReceived > 0) {
		// Loop through buffer attempting to decode packet until buffer is
		// exhausted or successful decode takes place
		for (i = 0; ((i < bufferLengthReceived) && (messageFound != 1)); i++) {
			messageFound = mavlink_parse_char(MAVLINK_COMM_0,              // Channel
											  buffer[i],                   // Char to parse
											  &incomingMessageHost,        // Message
											  &incomingMessageStatusHost); // Message status

			logPacket(incomingMessageHost); // Log reception of packet
		}
	} else {}
}
