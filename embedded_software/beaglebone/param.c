/**
 * @file param.c
 * @brief handle param packets
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 20, 2015
 */


#include "includes.h"

// Defines


// Function prototypes
void paramReceive(const mavlink_message_t *msg);

// Static variables

/**
 * Tell ground control that system has no editable params
 *
 * @param msg parameter related packet
 */
void paramReceive(const mavlink_message_t *msg) {
	mavlink_message_t replyMsg;

	if (((msg->msgid == MAVLINK_MSG_ID_PARAM_REQUEST_LIST) && (mavlink_msg_param_request_list_get_target_system(msg) == boatStateReturnSystemId())) ||
		((msg->msgid == MAVLINK_MSG_ID_PARAM_REQUEST_READ) && (mavlink_msg_param_request_read_get_target_system(msg) == boatStateReturnSystemId()))) {

		mavlink_msg_param_value_pack(boatStateReturnSystemId(),  // System ID
									 MAV_COMP_ID_SYSTEM_CONTROL, // Component
									 &replyMsg,                  // Message buffer
									 "NOPARAMS",                 // Param name
									 0,                          // Param value (dummy)
									 MAVLINK_TYPE_FLOAT,         // Param type
									 1,                          // Total params on system
									 0);                         // Current param index

		mavlinkWrapperSend(&replyMsg);
	} else {}
}
