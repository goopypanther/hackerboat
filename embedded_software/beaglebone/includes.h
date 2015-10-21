/**
 * @file includes.h
 * @brief main includes file
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @version 1.0
 * @license GPL 3.0
 * @since Jun 18, 2015
 */

#ifndef INCLUDES_H_
#define INCLUDES_H_

#define TRUE 1
#define FALSE 0

#include <mavlink.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <termios.h>
#include <sys/utsname.h>

#include "udp.h"
#include "log.h"
#include "args.h"
#include "mavlinkWrapper.h"
#include "Neo6m.h"
#include "uart.h"
#include "currentTime.h"
#include "Map.h"
#include "boatState.h"
#include "voltage.h"
#include "heartbeat.h"
#include "param.h"
#include "command.h"
#include "ioConfig.h"

#endif /* INCLUDES_H_ */
