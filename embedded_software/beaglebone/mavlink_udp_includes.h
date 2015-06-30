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
#include <math.h>

/* This assumes you have the mavlink headers on your include path
 or in the same folder as this source file */
#include <mavlink.h>

/** Position/velocity type
*
*/
typedef struct {
	float lat; /**< Float value of latitude */
	float lon; /**< Float value of longitude */
	int32_t alt; /**< Altitude */
	int16_t vx; /**< X velocity */
	int16_t vy; /**< Y velocity */
	int16_t vz; /**< Z velocity */
	uint16_t hdg; /**< Compass heading */
    uint8_t lock; /**< Gps lock achieved */
} position_t;

typedef struct {
    uint8_t *c;
    uint8_t len;
    int8_t dir;
} position_string_t;
