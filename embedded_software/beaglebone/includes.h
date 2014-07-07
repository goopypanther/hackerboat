#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
/* Linux / MacOS POSIX timer headers */
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>

/* This assumes you have the mavlink headers on your include path
 or in the same folder as this source file */
#include <mavlink.h>

typedef struct {
	int32_t lat;
	int32_t lon;
	int32_t alt;
	int16_t vx;
	int16_t vy;
	int16_t vz;
	uint16_t hdg;
} position_t;

typedef enum {IDLE, SENDING, REQUESTING} waypoint_state_t;
