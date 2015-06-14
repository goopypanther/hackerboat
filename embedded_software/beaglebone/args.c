/**
 * @file args.c
 * @brief Input argument parser
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 * @license GPL 3.0 
 * @version 1.0
 * @since 6/14/2015
 *
 * Parses and returns input arguments to the application.
 * Call parseInputParams() and then argsReturn functions will return pointers
 * to strings.
 */
 
#define STRING_BUFFER 100
#define DEFAULT_LOG_FILE "./mavlink_udp.log"

static char target_ip[STRING_BUFFER];
static char gps_serial_device[STRING_BUFFER];
static char low_level_debug_device[STRING_BUFFER];
static char low_level_serial_device[STRING_BUFFER];
static char log_file[STRING_BUFFER];

/**
 * @return string pointer of IP address of host
 */
char *argsReturnTargetIp(void) {
    return (&target_ip);
}

/**
 * @return string pointer of gps tty device
 */
char *argsReturnGpsSerialDevice(void) {
    return (&gps_serial_device);
}

/**
 * @return string pointer of debug tty device
 */
char *argsReturnLowLevelDebug(void) {
    return (&low_level_debug_device);
}

/**
 * @return string pointer of low level serial device
 */
char *argsReturnLowLevelSerial(void) {
    return (&low_level_serial_device);
}

/**
 * @return string pointer of log file
 */
char *argsReturnLogFile(void) {
    return (&log_file);
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
 * - 4. tty of debug interface
 * - 5. path to log file (optional)
 *
 * If incorrect number of params are passed, program will display help msg and
 * exit.
 *
 * @param argc Argument count
 * @param argv Char array of arguments entered
 */
void parseInputParams(int argc, char* argv[]) {
    // Check for proper number of arguments
    if (argc >= 5) {
        // Copy substrings to variables
        strncpy(target_ip, argv[1], STRING_BUFFER);
        strncpy(gps_serial_device, argv[2], STRING_BUFFER);
        strncpy(low_level_serial_device, argv[3], STRING_BUFFER);
        strncpy(low_level_debug_device, argv[4], STRING_BUFFER);
        
        // Make sure strings are still null terminated
        // (if buffer overrun occured)
        target_ip[STRING_BUFFER - 1] = 0x00;
        gps_serial_device[STRING_BUFFER - 1] = 0x00;
        low_level_debug_device[STRING_BUFFER - 1] = 0x00;
        low_level_serial_device[STRING_BUFFER - 1] = 0x00;

        // If log file was supplied
        if (argc == 6) {
            strncpy(log_file, argv[5], STRING_BUFFER);
            
        } else {
        // If no log file supplied
            strncpy(log_file, DEFAULT_LOG_FILE, STRING_BUFFER);
        }
        
        log_file[STRING_BUFFER - 1] = 0x00;
        
    } else {
    // Print help message if improper number of arguments passed
        printf("\n");
        printf("\tUsage:\n\n");
        printf("\t");
        printf("%s", argv[0]);
        printf(" <host ip> <GPS serial device> <low level serial device>\n");
        exit(EXIT_FAILURE);
    }
}
