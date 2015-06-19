/**
 * @file log.c
 * @brief Output lines to log
 *
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0 
 * @version 1.0
 * @since Jun 15, 2015
 *
 * Handles data output to logfile and stdout.
 * Call \c logOpen() to open log file and \c log() to send data to stdout
 * and logfile.
 */

#include "includes2.h"

// Static variables

static FILE *logFile;

static const char transmitedToHostString[] =     "Tx to host:       ";
static const char transmitedToLowLevelString[] = "Tx to low level:  ";
static const char receivedHostString[] =         "Rx from host:     ";
static const char receivedLowLevelString[] =     "Rx from low level:";

/**
 * Open log file
 *
 * @param logPath string pointer to log file
 */
void logOpen(const char *logPath) {
    logFile = fopen(logPath, "a"); // Open file, append mode
}

/**
 * Close log file
 */
void logClose(void) {
	(void) fclose(logFile);
}

/**
 * Prints data to standard out.
 *
 * @param data printf-compatible string & variables
 */
void logStdOut(const char *data, ...) {
	va_list dataList;

	va_start(dataList, data);
    vprintf(data, dataList);
    va_end(dataList);
}

/**
 * Prints data to standard out and log file.
 *
 * @param data printf-compatible string & variables
 */
void log(const char *data, ...) {
	va_list dataList;

	va_start(dataList, data);
    vprintf(data, dataList);
    va_end(dataList);

    va_start(dataList, data);
    vfprintf(logFile, data, dataList);
    va_end(dataList);
}

/**
 * Logs mavlink packet
 *
 * @param packet Mavlink packet to log
 */
void logPacket(mavlink_message_t *packet) {
	char *routing;
	char *packetType;

	// TODO Determine if packet is being transmitted or received
	if ()

	log("%s SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d %s", routing, packet->seq, packet->sysid, packet->compid, packet->len, packet->msgid, packetType);
}
