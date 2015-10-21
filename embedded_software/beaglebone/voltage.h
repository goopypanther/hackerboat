/**
 * @file voltage.h
 * @brief Read voltage level from batteries
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version 1.0
 * @since Jul 15, 2015
 */

#ifndef VOLTAGE_H_
#define VOLTAGE_H_

extern void voltageReceive(const mavlink_message_t *msg);
extern uint16_t voltageReturnBatteryVoltage(void);

#endif /* VOLTAGE_H_ */
