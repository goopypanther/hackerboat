/**
 * @file uart.h
 * @brief
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version
 * @since Jun 30, 2015
 */

#ifndef UART_H_
#define UART_H_

extern void uartInit(const char *gpsDevice, const char *lowLevelDevice);
extern void uartLowLevelSend(const uint8_t *data, uint32_t dataLength);
extern void uartGpsSend(const char *data);
extern uint32_t uartGetMessage(mavlink_message_t *message, mavlink_status_t *messageStatus);
extern uint8_t uartReturnIncomingNmeaChar(void);
extern void uartGpsLockMutex(void);
extern void uartGpsUnlockMutex(void);

#endif /* UART_H_ */
