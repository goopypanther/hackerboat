/**
 * @file mavlinkWrapper.h
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jun 18, 2015
 */

#ifndef MAVLINKWRAPPER_H_
#define MAVLINKWRAPPER_H_

extern void mavlinkWrapperSend(mavlink_message_t *packet);
extern uint32_t mavlinkWrapperReceive(void);
extern mavlink_message_t mavlinkWrapperReturnMessage(void);

#endif /* MAVLINKWRAPPER_H_ */
