/**
 * @file boatState.h
 * @brief nformation about boat type, mode, state, etc.
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 15, 2015
 */

#ifndef BOATSTATE_H_
#define BOATSTATE_H_

extern void boatStateReceive(const mavlink_message_t *msg);
extern void boatStateSetMode(MAV_MODE mode);
extern void boatStateSetState(MAV_STATE state);
extern MAV_MODE boatStateReturnMode(void);
extern uint32_t boatStateReturnCustomMode(void);
extern MAV_STATE boatStateReturnState(void);
extern MAV_TYPE boatStateReturnVehicleType(void);
extern MAV_AUTOPILOT boatStateReturnAutopilotType(void);
extern uint8_t boatStateReturnSystemId(void);
extern uint8_t boatStateReturnShoreSystemId(void);
extern uint8_t boatStateReturnLowLevelSystemId(void);
extern MAV_SYS_STATUS_SENSOR boatStateReturnSensors(void);

#endif /* BOATSTATE_H_ */
