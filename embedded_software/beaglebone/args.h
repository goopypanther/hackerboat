/**
 * @file args.h
 * @brief Input argument parser
 *
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0 
 * @version 1.0
 * @since Jun 15, 2015
 */

#ifndef ARGS_H_
#define ARGS_H_

extern char *argsReturnTargetIp(void);
extern char *argsReturnGpsSerialDevice(void);
extern char *argsReturnLowLevelDebug(void);
extern char *argsReturnLowLevelSerial(void);
extern char *argsReturnLogFile(void);
extern void parseInputParams(int argc, char* argv[]);

#endif /* ARGS_H_ */
