/**
 * @file args.c
 * @brief Input argument parser
 *
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0 
 * @version 1.0
 * @since Jun 15, 2015
 *
 * Parses and returns input arguments to the application.
 * Call \c parseInputParams() and then \c argsReturn functions will return
 * pointers to strings.
 */

#include "includes.h"

// Defines

#define STRING_BUFFER 1024
#define DEFAULT_LOG_FILE "./hackerboatNavigator.log"

// Static variables
static char target_ip[STRING_BUFFER];
static char gps_serial_device[STRING_BUFFER];
static char low_level_serial_device[STRING_BUFFER];
static char log_file[STRING_BUFFER];

/**
 * @return string pointer of IP address of host
 */
char *argsReturnTargetIp(void) {
    return (target_ip);
}

/**
 * @return string pointer of gps tty device
 */
char *argsReturnGpsSerialDevice(void) {
    return (gps_serial_device);
}

/**
 * @return string pointer of low level serial device
 */
char *argsReturnLowLevelSerial(void) {
    return (low_level_serial_device);
}

/**
 * @return string pointer of log file
 */
char *argsReturnLogFile(void) {
    return (log_file);
}

/**
 * Parses program input parameters
 *
 * An argument count of six is expected:
 *
 * - 0. Always program name
 * - 1. Target ipv4 of ground control station
 * - 2. tty of gps device
 * - 3. tty of low level control device
 * - 4. path to log file (optional)
 *
 * If incorrect number of params are passed, program will display help msg and
 * exit.
 *
 * @param argc Argument count
 * @param argv Char array of arguments entered
 */
void argsParseInputParams(int argc, char* argv[]) {
    // Check for proper number of arguments
    if ((argc >= 4) && (argc <= 5)) {
        // Copy substrings to variables
        strcpy(target_ip, argv[1]);
        strcpy(gps_serial_device, argv[2]);
        strcpy(low_level_serial_device, argv[3]);
        
        // Make sure strings are still null terminated
        // (if buffer overrun occurred)
        target_ip[STRING_BUFFER - 1] = 0x00;
        gps_serial_device[STRING_BUFFER - 1] = 0x00;
        low_level_serial_device[STRING_BUFFER - 1] = 0x00;

        // If log file was supplied
        if (argc == 5) {
            strcpy(log_file, argv[4]);
            
        } else {
        // If no log file supplied
            strcpy(log_file, DEFAULT_LOG_FILE);
        }
        
        log_file[STRING_BUFFER - 1] = 0x00;
        
    } else {
    // Print help message if improper number of arguments passed
        printf("\n Usage:\n\n %s", argv[0]);
        printf(" <host ip> <GPS serial device> <low level serial device> <optional path to log file>\n");
        exit(EXIT_FAILURE);
    }
}
