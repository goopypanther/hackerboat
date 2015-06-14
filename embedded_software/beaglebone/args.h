/**
 * @file args.h
 * @brief Input argument parser
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 * @license GPL 3.0 
 * @version 1.0
 * @since 6/14/2015
 */

extern char *argsReturnTargetIp(void);
extern char *argsReturnGpsSerialDevice(void);
extern char *argsReturnLowLevelDebug(void);
extern char *argsReturnLowLevelSerial(void);
extern char *argsReturnLogFile(void);
extern void parseInputParams(int argc, char* argv[]);
