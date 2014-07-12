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

typedef struct {
	float lat;
	float lon;
	int32_t alt;
	int16_t vx;
	int16_t vy;
	int16_t vz;
	uint16_t hdg;
    uint8_t lock;
} position_t;

typedef struct {
    uint8_t *c;
    uint8_t len;
    int8_t dir; 
} position_string_t;
