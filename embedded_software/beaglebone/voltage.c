/**
 * @file voltage.c
 * @brief Read voltage level from batteries
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 15, 2015
 */


#include "includes.h"

// Defines


// Function prototypes
void voltageReceive(const mavlink_message_t *msg);
uint16_t voltageReturnBatteryVoltage(void);

// Static variables
static uint16_t batteryVoltage = 0;

/**
 * Get battery voltage from battery status packet
 *
 * @param msg battery status packet
 */
void voltageReceive(const mavlink_message_t *msg) {
	// Check if battery packet
	if ((msg->msgid == MAVLINK_MSG_ID_BATTERY_STATUS) &&
		(msg->sysid == boatStateReturnLowLevelSystemId())) {

		batteryVoltage = mavlink_msg_battery_status_get_voltage_cell_1(msg);

	} else {}
}

/**
 * Return current battery voltage level
 *
 * @return battery voltage in millivolts
 */
uint16_t voltageReturnBatteryVoltage(void) {
	return (batteryVoltage);
}
