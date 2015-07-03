/**
 * @file uart.c
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 30, 2015
 */


#include "includes.h"

// Defines

// Function prototypes
void uartInit(const char *gpsDevice, const char *lowLevelDevice);
void uartLowLevelSend(const uint8_t *data, uint32_t dataLength);
void uartGpsSend(const char *data);
uint32_t uartGetMessage(mavlink_message_t *message, mavlink_status_t *messageStatus);
void *uartGpsReceiveThread(void);
void *uartLowLevelReceiveThread(void);

// Static variables
static int gpsFd;
static int lowLevelFd;

static uint32_t newMessage = FALSE;
static mavlink_message_t incomingMessage;
static mavlink_status_t incomingMessageStatus;

static pthread_mutex_t messageAccess;
static pthread_t gpsReceiveThread;
static pthread_t lowLevelReceiveThread;

/**
 * Initializes UART and opens devices
 *
 * @param gpsDevice path to tty
 * @param lowLevelDevice path to tty
 */
void uartInit(const char *gpsDevice, const char *lowLevelDevice) {
	// Open UARTS
	gpsFd = open(gpsDevice, O_RDWR);

	// Check for error opening GPS
	if (gpsFd < 0) {
		err(EXIT_FAILURE, "Failed to open GPS TTY %d\n", gpsFd);
	} else {}

	lowLevelFd = open(lowLevelDevice, O_RDWR);

	// Check for error opening low level device
	if (lowLevelFd < 0) {
		err(EXIT_FAILURE, "Failed to open low level TTY %d\n", lowLevelFd);
	} else {}

	// Set up threads & mutexes
	pthread_mutex_init(&messageAccess, NULL); // Initialize message mutex
	pthread_create(&gpsReceiveThread, NULL, (void *) uartGpsReceiveThread, NULL); // Start GPS thread
	pthread_create(&lowLevelReceiveThread, NULL, (void *) uartLowLevelReceiveThread, NULL); // Start low level thread
}

/**
 * Send data over UART to low level
 *
 * @param data Pointer to data byte array
 * @param dataLength Size of data byte array
 */
void uartLowLevelSend(const uint8_t *data, uint32_t dataLength) {
	write(lowLevelFd, data, dataLength);
}

/**
 * Send data to GPS over UART
 *
 * @param data null terminated string to send
 */
void uartGpsSend(const char *data) {
	write(gpsFd, data, strlen(data));
}

/**
 * Returns received message, will block until mutex lock acquired from
 * \c uartLowLevelReceiveThread
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
uint32_t uartGetMessage(mavlink_message_t *message, mavlink_status_t *messageStatus) {
	uint32_t packetReceived;

	pthread_mutex_lock(&messageAccess); // Acquire access to mavlink messages

	// Check if new packet was received
	if (newMessage) {
		newMessage = FALSE; // Reset message flag

		// Copy messages
		*message = incomingMessage;
		*messageStatus = incomingMessageStatus;

		packetReceived = TRUE;

	} else {
	// If no new packet has been received
		packetReceived = FALSE;
	}

	pthread_mutex_unlock(&messageAccess);// Release access to mavlink messages

	return (packetReceived);
}

/**
 * Receives NMEA strings over UART
 * Each char is passed to Neo6m module for parsing.
 */
void *uartGpsReceiveThread(void) {
	for (;;) {

	}

	return (NULL);
}

/**
 * Receives mavlink packets over UART
 * Each char is passed to mavlink module for decode.
 */
void *uartLowLevelReceiveThread(void) {
	for (;;) {

	}
	return (NULL);
}
