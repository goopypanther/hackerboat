/**
 * @file boatState.c
 * @brief Information about boat type, mode, state, etc.
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 14, 2015
 */


#include "includes.h"

// Defines

// Function prototypes
void boatStateReceive(const mavlink_message_t *msg);
void boatStateSetMode(MAV_MODE mode);
void boatStateSetState(MAV_STATE state);
MAV_MODE boatStateReturnMode(void);
uint32_t boatStateReturnCustomMode(void);
MAV_STATE boatStateReturnState(void);
MAV_TYPE boatStateReturnVehicleType(void);
MAV_AUTOPILOT boatStateReturnAutopilotType(void);
uint8_t boatStateReturnSystemId(void);
uint8_t boatStateReturnShoreSystemId(void);
uint8_t boatStateReturnLowLevelSystemId(void);
MAV_SYS_STATUS_SENSOR boatStateReturnSensors(void);

// Static variables
static MAV_MODE boatMode = MAV_MODE_PREFLIGHT;
static MAV_STATE boatState = MAV_STATE_BOOT;

/**
 * Set boat mode from incoming mode set packet
 *
 * @param msg pointer to mavlink packet
 */
void boatStateReceive(const mavlink_message_t *msg) {
	MAV_MODE mode;

	// If packet is set mode type && is addressed to our system && system has booted
	if ((boatState != MAV_STATE_BOOT) &&
		(msg->msgid == MAVLINK_MSG_ID_SET_MODE) &&
		(mavlink_msg_set_mode_get_target_system(msg) == boatStateReturnSystemId())) {

		mode = mavlink_msg_set_mode_get_base_mode(msg);

		// We don't actually have manual stabilized mode, switch that to normal
		// manual mode
		switch (mode) {
		case MAV_MODE_STABILIZE_ARMED:
			mode = MAV_MODE_MANUAL_ARMED;
			break;

		case MAV_MODE_STABILIZE_DISARMED:
			mode = MAV_MODE_MANUAL_DISARMED;
			break;

		default:
			break;
		}

		boatStateSetMode(mode);

	} else {}
}

/**
 * Set boat mode
 *
 * @param mode mode to enter
 */
void boatStateSetMode(MAV_MODE mode) {
	boatMode = mode;
}

/**
 * Set boat state
 *
 * @param state state to enter
 */
void boatStateSetState(MAV_STATE state) {
	boatState = state;
}

/**
 * Return mode
 *
 * @return current mode
 */
MAV_MODE boatStateReturnMode(void) {
	return (boatMode);
}

/**
 * Return custom mode
 *
 * @return custom mode
 */
uint32_t boatStateReturnCustomMode(void) {
	return (0);
}

/**
 * Return state
 *
 * @return current state
 */
MAV_STATE boatStateReturnState(void) {
	return (boatState);
}

/**
 * Returns type of vehicle
 *
 * @return type of vehicle
 */
MAV_TYPE boatStateReturnVehicleType(void) {
	return (MAV_TYPE_SURFACE_BOAT);
}

/**
 * Returns type of autopilot
 *
 * @return autopilot type
 */
MAV_AUTOPILOT boatStateReturnAutopilotType(void) {
	return (MAV_AUTOPILOT_GENERIC);
}

/**
 * Returns system ID of low level
 *
 * @return low level system id
 */
uint8_t boatStateReturnLowLevelSystemId(void) {
	return (2);
}

/**
 * Returns system ID of boat
 *
 * @return system id
 */
uint8_t boatStateReturnSystemId(void) {
	return (1);
}

/**
 * Returns system ID of shore station
 *
 * @return shore system id
 */
uint8_t boatStateReturnShoreSystemId(void) {
	return (255);
}
/**
 * Returns boat sensors
 *
 * @return sensors attached to boat
 */
MAV_SYS_STATUS_SENSOR boatStateReturnSensors(void) {
	return (MAV_SYS_STATUS_SENSOR_3D_GYRO |
			MAV_SYS_STATUS_SENSOR_3D_ACCEL |
			MAV_SYS_STATUS_SENSOR_3D_MAG |
			MAV_SYS_STATUS_SENSOR_GPS |
			MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS);
}
