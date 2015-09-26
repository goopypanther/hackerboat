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
void mavlinkWrapperSend(mavlink_message_t *packet);
uint32_t mavlinkWrapperReceive(void);
mavlink_message_t mavlinkWrapperReturnMessage(void);

// Static variables
static mavlink_message_t incomingMessage;
static mavlink_status_t incomingMessageStatus;

/**
 * Sends mavlink packet over UDP and UART
 *
 * @param packet Mavlink packet to send
 */
void mavlinkWrapperSend(mavlink_message_t *packet) {
	char packetBuffer[sizeof(mavlink_message_t)];
	uint32_t packetBufferFilled;

	packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, packet); // Copy packet to buffer

	udpSend(packetBuffer, packetBufferFilled); // Send contents of buffer over UDP
	uartLowLevelSend(packetBuffer, packetBufferFilled); // Send contents of buffer over UART

	logLine("Transmitted:");
	logPacket(packet); // Log transmission of packet
}

/**
 * Receive and decode mavlink packets from UDP and UART
 *
 * @return 1 if new packet received, 0 if no new packet received
 */
uint32_t mavlinkWrapperReceive(void) {
    char packetBuffer[sizeof(mavlink_message_t)];
    uint32_t packetBufferFilled;
	uint32_t messageFound;

	messageFound = FALSE; // Set initial state

	// First, try to receive packet from UDP, these may be important
	// commands from ground control.
	messageFound = udpGetMessage(&incomingMessage, &incomingMessageStatus); // Receive packet from UDP socket

	// Check if packet found
	if (messageFound == TRUE) {
		logLine("Received UDP, forward to UART:");
		uartLowLevelSend(packetBuffer, packetBufferFilled); // Sent contents of buffer over UART

	} else {
	// Second, try to receive packet from UART
		messageFound = uartGetMessage(&incomingMessage, &incomingMessageStatus); // Receive packet from UART

		// Check if packet found
		if (messageFound == TRUE) {
			logLine("Received UART, echo to UDP:");

			// Echo received UART packet over UDP
			packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &incomingMessage);
			udpSend(packetBuffer, packetBufferFilled); // Send contents of buffer over UDP
		} else {}
	}

	// Log reception of packet if complete packet received
	if (messageFound) {
		logPacket(&incomingMessage);
	} else {}

	return (messageFound);
}

/**
 * Returns most recently received mavlink packet
 *
 * @return most recently received packet
 */
mavlink_message_t mavlinkWrapperReturnMessage(void) {
	return (incomingMessage);
}
