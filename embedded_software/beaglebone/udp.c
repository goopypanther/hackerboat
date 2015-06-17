/**
 * @file udp.c
 * @brief UDP connection
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 16, 2015
 *
 */

#include "udp.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Defines
#define LOCAL_PORT 14551
#define HOST_PORT  14550
#define RECEIVE_BUFFER_LENGTH 500

// Function prototypes


// Static variables

static int socket;
static struct sockaddr_in hostAddress;
static uint8_t receiveBuffer[RECEIVE_BUFFER_LENGTH];

/**
 * Open UDP socket
 *
 * @param host Address to connect to
 */
void udpOpenSocket(const char *host) {
	int returnValue;
	struct sockaddr_in localAddress;

    memset(&hostAddress, 0, sizeof(hostAddress)); // Zero out struct

    gcAddr.sin_addr.s_addr = inet_addr(host);
    gcAddr.sin_family = AF_INET;
    gcAddr.sin_port = htons(HOST_PORT);

	memset(&localAddress, 0x00, sizeof(localAddress)); // Zero out struct

	// Socket settings
	localAddress.sin_addr.s_addr = INADDR_ANY;
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(LOCAL_PORT);

	socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create socket

	// Catch socket creation failure
	if (socket < 0) {
		err(EXIT_FAILURE, "Failed to open socket.");

	} else {}

	// Bind socket to port
	returnValue = bind(socket,                           // Socket identifier
					   (struct sockaddr*) &localAddress, // Settings struct
					   sizeof(localAddress));            // Size of settings struct

	// Catch bind failure
	if (returnValue < 0) {
		close(socket);

		err(EXIT_FAILURE, "Failed to bind socket.");

	} else {}

    returnValue = fcntl(socket,	F_SETFL, O_NONBLOCK); // Make socket non-blocking

    // Catch non-blocking failure
    if (returnValue < 0) {
        close(socket);

        err(EXIT_FAILURE, "Failed setting non-blocking.");

    } else {}

}

/**
 * Close UDP socket
 */
void udpCloseSocket(void) {
	close(socket);
}

/**
 * Send data over UDP socket to host
 *
 * @param data Pointer to data byte array
 * @param dataLength Size of data byte array
 */
void udpSend(const uint8_t *data, uint32_t dataLength) {
	sendto(socket,                       	   // UDP socket
		   data,                               // Data buffer
		   dataLength,                         // Data length
		   0,                                  // Flags
		   (struct sockaddr_in*) &hostAddress, // Address
		   sizeof(struct sockaddr_in));        // Address length
}

/**
 * Receive data from UDP socket
 *
 * @param buf Char pointer to received data buffer
 * @return Number of bytes received
 */
uint32_t udpReceive(char *buf) {
	uint32_t returnLength;

	returnLength = recvfrom(socket,                 // Socket device
					   	    (void *) receiveBuffer, // Receive buffer
					   	    RECEIVE_BUFFER_LENGTH,  // Length of buffer
					   	    0,                      // Extra settings (none)
					   	    0,						// Ignore receive from address
					   	    0);                     // Ignore receive address length

	return (returnLength);
}
