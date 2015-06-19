/**
 * @file log.h
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 15, 2015
 */

#ifndef LOG_H_
#define LOG_H_

extern void logOpen(const char *logPath);
extern void logClose(void);
extern void logStdOut(const char *data, ...);
extern void log(const char *data, ...);

#endif /* LOG_H_ */
