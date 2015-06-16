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
 * Call logOpen() to open log file and log() to send data to stdout and logfile.
 */

#include "includes.h"

static FILE *logFile;

/**
 * logOpen
 *
 * @param logPath string pointer to log file
 */
void logOpen(const char *logPath) {
    logFile = fopen(logPath, "a"); // Open file, append mode
}

/**
 * logStdOut
 *
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
 * log
 *
 * Prints data to standard out and log file.
 *
 * @param data printf-compatible string & variables
 */
void log(const char *data, ...) {
	va_list dataList;

	va_start(dataList, data);
    vprintf(data, dataList);
    va_end(dataList);

    va_start(dataList);
    vfprintf(logFile, data, dataList);
    va_end(dataList);
}
