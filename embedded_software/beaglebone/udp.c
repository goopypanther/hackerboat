/**
 * @file udp.c
 * @brief UDP connection functions
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 16, 2015
 *
 * Call \c udpOpenSocket() first, then \c udpSend() and \c udpReceive() as
 * needed.
 */

#include "includes.h"

// Defines
#define LOCAL_PORT 14551
#define HOST_PORT  14550

#define BUFFER_LENGTH 2048

// Function prototypes
void udpOpenSocket(const char *host);
void udpCloseSocket(void);
void udpSend(const uint8_t *data, uint32_t dataLength);
uint32_t udpReceive(char *buf, uint32_t bufLen);
uint32_t udpGetMessage(mavlink_message_t *message, mavlink_status_t *messageStatus);

// Static variables

static int socketDevice;
static struct sockaddr_in hostAddress;

/**
 * Open UDP socket
 *
 * @param host Address to connect to
 */
void udpOpenSocket(const char *host) {
	int returnValue;
	struct sockaddr_in localAddress;

    memset(&hostAddress, 0, sizeof(hostAddress)); // Zero out struct

    hostAddress.sin_addr.s_addr = inet_addr(host);
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_port = htons(HOST_PORT);

	memset(&localAddress, 0x00, sizeof(localAddress)); // Zero out struct

	// Socket settings
	localAddress.sin_addr.s_addr = INADDR_ANY;
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(LOCAL_PORT);

	socketDevice = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create socket

	// Catch socket creation failure
	if (socketDevice < 0) {
		err(EXIT_FAILURE, "Failed to open socket.");

	} else {}

	// Bind socket to port
	returnValue = bind(socketDevice,                     // Socket identifier
					   (struct sockaddr*) &localAddress, // Settings struct
					   sizeof(localAddress));            // Size of settings struct

	// Catch bind failure
	if (returnValue < 0) {
		close(socketDevice);

		err(EXIT_FAILURE, "Failed to bind socket.");

	} else {}

    returnValue = fcntl(socketDevice, F_SETFL, O_NONBLOCK); // Make socket non-blocking

    // Catch non-blocking failure
    if (returnValue < 0) {
        close(socketDevice);

        err(EXIT_FAILURE, "Failed setting non-blocking.");

    } else {}

}

/**
 * Close UDP socket
 */
void udpCloseSocket(void) {
	close(socketDevice);
}

/**
 * Send data over UDP socket to host
 *
 * @param data Pointer to data byte array
 * @param dataLength Size of data byte array
 */
void udpSend(const uint8_t *data, uint32_t dataLength) {
	// Send buffer over UDP socket
	sendto(socketDevice,                   	   // UDP socket
		   data,                               // Data buffer
		   dataLength,                         // Data length
		   0,                                  // Flags
		   (struct sockaddr*) &hostAddress, // Address
		   (socklen_t) sizeof(struct sockaddr_in));        // Address length
}

/**
 * Receive data from UDP socket
 *
 * @param buf Char pointer to received data buffer
 * @param bufLen Size of buffer
 * @return Number of bytes received
 */
uint32_t udpReceive(char *buf, uint32_t bufLen) {
	int32_t returnLength;

	// Receive data from UDP socket into buffer
	returnLength = (int32_t) recvfrom(socketDevice, // Socket device
					   	              (void *) buf, // Receive buffer
					   	              bufLen,       // Length of buffer
					   	              0,            // Extra settings (none)
					   	              0,            // Ignore receive from address
					   	              0);           // Ignore receive address length

	// Check for errors
	if (returnLength < 0) {
		returnLength = 0;
	} else {}

	return ((uint32_t) returnLength);
}

/**
 * Receives mavlink packet from UDP and decodes
 *
 * @param message pointer to mavlink message received packet will be stored
 * into
 *
 * @param messageStatus pointer to message status struct received message will
 * be stored into
 *
 * @return 0 if no new packet received (\c message and \c messageStatus will
 * not be altered), 1 if new packet received, \c message and \c messageStatus
 * will be updated)
 */
uint32_t udpGetMessage(mavlink_message_t *message, mavlink_status_t *messageStatus) {
	char buffer[BUFFER_LENGTH];
	uint32_t bufferLengthReceived;
	uint32_t i;
	uint32_t messageFound;

	messageFound = FALSE; // Set initial state

	bufferLengthReceived = udpReceive(buffer, sizeof(buffer)); // Receive packet from UDP socket

	// Check if something was received
	if (bufferLengthReceived > 0) {

		// Loop through buffer attempting to decode packet until buffer is
		// exhausted or successful decode takes place. messageFound will become
		// TRUE if a packet has been decoded.
		for (i = 0; ((i < bufferLengthReceived) && (messageFound == FALSE)); i++) {
			messageFound = mavlink_parse_char(MAVLINK_COMM_0, // Channel (different than UART chan)
											  buffer[i],      // Char to parse
											  message,        // Message
											  messageStatus); // Message status
		}

	}

	return (messageFound);
}
