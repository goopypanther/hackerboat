/**
 * @file heartbeat.c
 * @brief tracks heartbeat packets
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 18, 2015
 */

#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_

extern void heartbeatReceive(const mavlink_message_t *msg);
extern void heartbeatSend(void);
extern uint32_t heartbeatCheckPanicHost(void);
extern uint32_t heartbeatCheckPanicLowLevel(void);

#endif /* HEARTBEAT_H_ */
