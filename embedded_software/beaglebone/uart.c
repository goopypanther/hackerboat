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
uint8_t uartReturnIncomingNmeaChar(void);
void uartGpsLockMutex(void);
void uartGpsUnlockMutex(void);

// Static variables
static int gpsFd;
static int lowLevelFd;

static uint32_t newMessage = FALSE;
static mavlink_message_t incomingMessage;
static mavlink_status_t incomingMessageStatus;

static pthread_mutex_t messageAccess;
static pthread_t gpsReceiveThread;
static pthread_t lowLevelReceiveThread;

static pthread_mutex_t nmeaAccess;
static uint8_t incomingNmeaChar;

/**
 * Initializes UART and opens devices
 *
 * @param gpsDevice path to tty
 * @param lowLevelDevice path to tty
 */
void uartInit(const char *gpsDevice, const char *lowLevelDevice) {
	struct termios lowLevelFdSettings;

	// Open UARTS
	gpsFd = open(gpsDevice, O_RDWR | O_NONBLOCK | O_NDELAY);

	// Check for error opening GPS
	if (gpsFd < 0) {
		err(EXIT_FAILURE, "Failed to open GPS TTY %d\n", gpsFd);
	} else {}

	lowLevelFd = open(lowLevelDevice, O_RDWR | O_NOCTTY | O_NONBLOCK);

	// Check for error opening low level device
	if (lowLevelFd < 0) {
		err(EXIT_FAILURE, "Failed to open low level TTY %d\n", lowLevelFd);
	} else {}

	// Set lowLevelFd into raw mode
	tcgetattr(lowLevelFd, &lowLevelFdSettings);
	cfmakeraw(&lowLevelFdSettings);
	tcsetattr(lowLevelFd, TCSANOW, &lowLevelFdSettings);

	Neo6mInit(); // Configure GPS and parsing functions

	// Set up mutexes
	pthread_mutex_init(&messageAccess, NULL);
	pthread_mutex_init(&nmeaAccess, NULL);

	// Set up threads
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

	pthread_mutex_unlock(&messageAccess);// Release access to mavlink message

	return (packetReceived);
}

/**
 * Receives NMEA strings over UART
 * Each char is passed to Neo6m module for parsing.
 */
void *uartGpsReceiveThread(void) {
	for (;;) {
		read(gpsFd, &incomingNmeaChar, sizeof(incomingNmeaChar)); // Get char from UART

		uartGpsLockMutex(); // Acquire lock on mutex

		Neo6mMeldDataISR(); // Add char to buffer
		Neo6mParseBuffer(); // Parse buffer

		uartGpsUnlockMutex(); // Release mutex
	}

	return (NULL);
}

/**
 * Locks mutex to allow NMEA module to update data
 */
void uartGpsLockMutex(void) {
	pthread_mutex_lock(&nmeaAccess);
}

/**
 * Unlocks mutex for NMEA module
 */
void uartGpsUnlockMutex(void) {
	pthread_mutex_unlock(&nmeaAccess);
}

/**
 * Passthrough function for NMEA module
 *
 * @return most recent NMEA char received
 */
uint8_t uartReturnIncomingNmeaChar(void) {
	return (incomingNmeaChar);
}

/**
 * Receives mavlink packets over UART
 *
 * Loop waits to receive char and passes it to mavlink for decode. When decode
 * succeeds thread acquires mutex and copies received message to static
 * variables.
 */
void *uartLowLevelReceiveThread(void) {
	uint32_t messageFound;
	uint8_t incomingChar;
	mavlink_message_t message;
	mavlink_status_t messageStatus;

	// Main loop, runs forever
	for (;;) {
		read(lowLevelFd, &incomingChar, sizeof(incomingChar)); // Get char from UART

		messageFound = mavlink_parse_char(MAVLINK_COMM_1,  // Channel (different than UDP chan)
										  incomingChar,    // Char to parse
										  &message,        // Message
										  &messageStatus); // Message status

		// If message was decoded
		if (messageFound) {
			pthread_mutex_lock(&messageAccess); // Acquire access to mavlink messages

			newMessage = TRUE; // Tell uartGetMessage a new message has arrived

			// Copy received messages to static variables
			incomingMessage = message;
			incomingMessageStatus = messageStatus;

			pthread_mutex_unlock(&messageAccess);// Release access to mavlink messages

		} else {}
	}

	return (NULL);
}
