//#define _POSIX_SOURCE
//#define __USE_XOPEN2KXSI
//#define __USE_XOPEN

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
//#include <posix.h>
//#include <pty.h>
#include "checksum.h"

#define PILOT_TTY	"/dev/ttyO1"
#define SOCAT_IN	stdin
#define SOCAT_OUT	stdout
#define LOG_PATH	"/home/root/StalkerCopter/log/"
#define LOG_CMD		"commands.log"
#define LOG_TELEX	"telemetry.log"
//#define PTTY		"/dev/ptyp00"

#define ROLLGAIN		(200)
#define PITCHGAIN		(200)
#define YAWGAIN			(200)
#define THROTTLEGAIN	(200)
#define MODCMDLEN		(8)					// number of channels in a MAVLink RC override packet
#define MODPKTSCANF		 "%f,%f,%f,%f\n"	// format string for sucking down the mod packet		
#define MODROLLOFFSET	(0)	
#define MODPITCHOFFSET	(1)	
#define MODYAWOFFSET	(2)	
#define MODTHRTLOFFSET	(3)
#define MAXRCCMD		(2000)
#define MINRCCMD		(2000)
		

#define BUFSIZE		280

int getLastIndex(char * str, int size);
int initSerial (FILE * serial);
int initStdio (void);
int initPty (void);
int getPacket (FILE * stream, unsigned char * buf, int * index, int * len);
int eatPacket (unsigned char *inbuf, unsigned char *packetbuf, int *bufIndex, int bufLen, int *packetIndex, int *packetLen);
int transmitPacket (FILE * stream, FILE * log, unsigned char *buf, int len);
int reportPacket (FILE *log, unsigned char *buf, int len, const char *source);
int procCmdPacket (unsigned char *packetbuf, int packetlen, int *trackFlag, int16_t *commands);
inline int getModPacket (FILE *mod, int16_t *commands);
int procRCOverridePacket (unsigned char *packetbuf, int packetlen, int16_t *commands);
inline float boundValueFloat (float val, float upper, float lower);
inline uint16_t boundValueUInt16 (uint16_t val, uint16_t upper, uint16_t lower);
int writeLEDs(int val);

int main (void) {
	unsigned char serialBuf[BUFSIZE*10];			// input buffer for bytes read from the serial port (i.e. Ardupilot)
	unsigned char stdinBuf[BUFSIZE*10];			// input buffer for bytes read from the ground station via stdin (i.e. via socat)
	unsigned char telemetryPacket[BUFSIZE];		// buffer for storing the current telemetry packet
	unsigned char commandPacket[BUFSIZE];		// buffer for storing the current command packet
	//char modPacket[BUFSIZE];			// buffer for storing the current modification packet
	char timestr[BUFSIZE];				// buffer for storing a time string for logging
	char cmdlogname[BUFSIZE] = LOG_PATH;	// buffer for storing the full path of the command log
	char tellogname[BUFSIZE] = LOG_PATH;	// buffer for storing the full path of the telemetry log
	int telemetryIndex = 0;				// index of the current byte in the current telemetry packet
	int commandIndex = 0;				// index of the current byte in the current command packet
	//int modLen = 0; 					// length of the current modification packet
	int telemetryLen = 8;				// length of the current telemetry packet (initialized to 8, since it's the header length of a MAVLink packet)
	int commandLen = 8;					// length of the current command packet (initialized to 8, since it's the header length of a MAVLink packet)
	int serialIndex = 0;				// index of the current byte in serialBuf
	int stdinIndex = 0;					// index of the current byte in stdinbuf
	int telemetryCnt;					// number of bytes in serialBuf
	int commandCnt; 					// number of bytes in stdinBuf
	int modFD/*, slaveFD*/;					// file descriptors for the pseudo-TTY
	unsigned char *slave;						// name of the pseudo-TTY slave
	int trackFlag = 0;					// flag to indicate status of tracking
	FILE *out;							// serial port file handle, for connection to the Ardupilot
	FILE *inlog, *outlog;				// log file handles	
	FILE *mod;							// file handle 
	int telemetryFD, commandFD;			// log file descriptors
	int16_t modCommands[MODCMDLEN] = {0,0,0,0,0,0,0,0};	// array of RC override commands
	time_t rawtime;						// variable to hold the current time	
	int terms = 0;						// number of terms grabbed from last call to getModPacket()

	out = fopen(PILOT_TTY, "r+");		// opening serial port
	initSerial(out);					// configure serial port
	initStdio();						// configure stdin/stdout
	modFD = initPty();					// open and configure pseudo TTY for IPC
	slave = ptsname(modFD);				// get the name of the associated slave
	mod = fdopen(modFD, "r+");			// associate a stream with the pseudo TTY
	telemetryFD = fileno(out);
	commandFD = fileno(stdin);
	
	time(&rawtime);														// get the time
	strftime(timestr, BUFSIZE, "%H-%M-%S-", localtime(&rawtime));		// create a useful string of the current time (no battery-backed RTC, so date is useless)
	strcat(cmdlogname, timestr); strcat(cmdlogname, LOG_CMD);			// assemble the name of the command log
	strcat(tellogname, timestr); strcat(tellogname, LOG_TELEX);			// assemble the name of the telemetry log
	inlog = fopen(cmdlogname, "w");										// open command log
	if (!inlog) {														// check status of command log opening
		fprintf(stderr, "Command log failed to open... %d\n", errno);
		perror("Command Log");
	}
	outlog = fopen(tellogname, "w");									// open telemetry log
	if (!outlog) {														// check status of telemetry log opening			
		fprintf(stderr, "Telemetry log failed to open... %d\n", errno);
		perror("Telemetry Log");
	}
	
	fprintf(stderr, "started... %d,%d\n", fileno(inlog), fileno(outlog));		// print startup status
	perror("stalkerLink");
	
	while (1) {
		// clear input buffers
		memset(serialBuf,'\0',(BUFSIZE*10));							
		memset(stdinBuf,'\0',(BUFSIZE*10));
		serialIndex = 0; stdinIndex = 0;
		
		// fill input buffers 
		telemetryCnt = read(telemetryFD, serialBuf, (BUFSIZE*10));
		commandCnt = read(commandFD, stdinBuf, (BUFSIZE*10));
		
		// process contents of input buffers
		while ((serialIndex < telemetryCnt) || (stdinIndex < commandCnt)) {			// iterate while there are bytes to process
			if (eatPacket(stdinBuf, commandPacket, &stdinIndex, commandCnt, &commandIndex, &commandLen) == 1) {		// process a byte from the command buffer, do stuff if we've got a whole packet
				if ((terms = getModPacket(mod, modCommands)) > 0) {					// grab a modification command from the pseudo-TTY
					fprintf(stderr, "Got %d terms from last command packet:\t%d\t%d\t%d\t%d\n",
						terms, (modCommands[MODROLLOFFSET]), (modCommands[MODPITCHOFFSET]), 
						(modCommands[MODYAWOFFSET]), (modCommands[MODTHRTLOFFSET]));
				}
				procCmdPacket(commandPacket, commandLen, &trackFlag, modCommands);	// modify the command packet, if it's the right type
				transmitPacket(out, inlog, commandPacket, commandLen);				// transmit the command packet to the Ardupilot & log it
			}
			if (eatPacket(serialBuf, telemetryPacket, &serialIndex, telemetryCnt, &telemetryIndex, &telemetryLen) == 1) {	// process a byte from the telemetry buffer, do stuff if we've got a whole packet
				transmitPacket(SOCAT_OUT, outlog, telemetryPacket, telemetryLen);	// transmit the telemetry packet to the ground station & log it
			}
		}
	}

	fprintf(stderr, "Dropped out the bottom\n");	// if we should somehow break out, it would be nice to know about it.
	
}

int eatPacket (unsigned char *inbuf, unsigned char *packetbuf, int *bufIndex, int bufLen, int *packetIndex, int *packetLen) {
	char inbyte;
	time_t rawtime;
	char timestr[BUFSIZE];
	//int i = 0;
	
	if ((*bufIndex) >= bufLen) return -1;	// make sure we haven't reached the end of the input buffer (return failure if we have)
	inbyte = inbuf[(*bufIndex)];			// grab a single byte from the buffer
	(*bufIndex)++;							// advance the pointer
	
	if (0 == (*packetIndex)) {							// these are things we do if it's the first byte
		if ((0x55 == inbyte) || (0x70 == inbyte)) {		// check that it's a valid first byte
			memset(packetbuf,'\0',BUFSIZE);				// if it is, clear the packet
			packetbuf[(*packetIndex)] = inbyte;			// write the byte
			(*packetIndex)++;							// advance the pointer
			if (0x70 == inbyte) { 						// if it's a ground station calling packet,
				(*packetLen) = 13;						// we know it's 13 bytes long
				return 0;								// return success
			} else if (0x55 == inbyte) { 				// if it's a normal packet
				(*packetLen) = 8;						// we know the header & CRC are 8 bytes long
				return 0;								// return success
			}
		} else {										// if we ever get anything other than 0x55 or 0x70 here, it's an error
			time(&rawtime);								// so we report it
			strftime(timestr, BUFSIZE, "%M,%S", localtime(&rawtime));
			fprintf(stderr, "Got anomalous byte at index 0: 0x%x, %s\n", inbyte, timestr);
			return -1;									// and return failure
		}
	} else {
		packetbuf[(*packetIndex)] = inbyte;				// write the byte into the packet
		if ((packetbuf[0] == 0x55) && (1 == (*packetIndex))) {(*packetLen) += inbyte;}	//if it's byte 1 of a normal packet, this the payload length, so add it to the packet length
		(*packetIndex)++;								// advance the pointer
		if ((*packetIndex) >= (*packetLen)) {			// if we have reached the end of the packet
			(*packetIndex) = 0;							// set the pointer back to zero, so the next iteration starts in the right place
			return 1;									// return end of packet
		} 
	}
	return 0;
	
}

int transmitPacket (FILE *stream, FILE * log, unsigned char *buf, int len) {
	int i;
	time_t rawtime;
	char timestr[BUFSIZE];
	int fd;
	
	fd = fileno(stream);	// grab the file descriptor of the input stream
	write(fd, buf, len);	// write the packet to the file descriptor
	
	time(&rawtime);														// get the time for logging
	strftime(timestr, BUFSIZE, "%M,%S", localtime(&rawtime));			// assemble the time string
	fprintf(log, "%s", timestr);										// print it to the log file
	fprintf(log, ",%d", len);											// print the length of the packet	
	fprintf(log, ",0x%x", crc_calculate((uint8_t *)(buf+1), (len-3)));  // calculate & print the CRC in hex
	for (i=0; i<len; i++) {		// print the bytes of the packet in hex format to the log						
		fprintf(log, ",0x%x", buf[i]);
	}
	if ((crc_calculate((uint8_t *)(buf+1), (len-3))) != 				// check the CRC, and print it in decimal at the end of the line
		((uint16_t)(buf[(len-2)])+((uint16_t)(buf[(len-1)])*256))) {	// this aids correlating bad packets with the ground station console
		fprintf(log, ",*%d", (crc_calculate((uint8_t *)(buf+1), (len-3))));
	} 
	fprintf(log, "\n");			// end the line
	fflush(log);				// flush the log buffer
	return errno;				// return the error code, if any
}

int initStdio (void) {
	int fd, flags;

	setvbuf(stdout, NULL, _IONBF, 0);	// setting no buffering on stdout	
	fd = fileno(stdin);					// grab the file descriptor for stdin so we can set flags on it
	flags = fcntl(fd, F_GETFL, 0);		// get the fcntl flags
	flags |= O_NONBLOCK;				// set the fcntl flags to non-blocking 
	fcntl(fd, F_SETFL, flags);			// write the fcntl flags
	return errno;
}

int initPty (void) {
	int fd, flags;
	struct termios options;
	
	fd = posix_openpt(O_RDWR|O_NOCTTY);		// create new pseudo TTY pair
	if ((fd < 0) || 
		(grantpt (fd) < 0) || 				// check that we can grant access
		(unlockpt (fd) < 0) || 				// check that we can unlock it
		(ptsname (fd) == NULL)) {			// check that the slave exists
		perror("Pseudo-TTY");
	}
	
	// make it non-blocking
	flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
	
	//make it canonical
	tcgetattr(fd, &options);
	options.c_lflag &= ICANON;
	if (tcsetattr(fd, TCSANOW, &options)) {
		fprintf(stderr, "Pseudo TTY settings failed\n");
		perror("Pseudo-TTY");
	}
	
	return fd;
}

int initSerial (FILE *serial) {
	int fd, flags;
	struct termios options;

	// and here we go to a whole lot of trouble to set up the serial port
	// just so in order to get reliable comms
	fd = fileno(serial);
	
	// start by setting the fcntl flags
	flags = fcntl(fd, F_GETFL, 0);
	flags |= (O_NONBLOCK | O_RDWR | O_NOCTTY );				// non-blocking mode, read-write, not controlling TTY
	fcntl(fd, F_SETFL, flags);
	
	// then set the serial options
	tcgetattr(fd, &options);								// get the old settings
	cfsetispeed(&options, B57600);							// set input baud rate
	cfsetospeed(&options, B57600);							// set output baud rate
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);			// no parity, one stop bit
	options.c_cflag |= (CLOCAL | CREAD | CS8);				// ignore modem status lines, enable receiver, 8 bits per byte
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);		// make sure we are in raw mode
	options.c_iflag &= ~(IXON | IXOFF | IXANY);				// turn off all flow control
	options.c_iflag &= ~(ICRNL);							// turn off line ending translation					
	options.c_oflag &= ~(OPOST);							// turn off post processing of output
	options.c_cc[VMIN] = 0;									// this sets the timeouts for the read() operation to minimum
	options.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSANOW, &options)) {					// write the new settings, check success & report
		fprintf(stderr, "serial settings failed\n");
		perror("stalkerLink");
	}
	
	return errno;
}

int procCmdPacket (unsigned char *packetbuf, int packetlen, int *trackFlag, int16_t *commands) {
	switch (packetbuf[5]) {			// switch on the MAVLink message ID
		case 70:	// RC_CHANNELS_OVERRIDE
			fprintf(stderr, "Got RC_CHANNELS_OVERRIDE packet\n");
			if (*trackFlag) {		// if the trackFlag is true, mix the commands
				procRCOverridePacket(packetbuf, packetlen, commands);
			}
			break;
		case 176:	// MAV_CMD_DO_SET_MODE
			fprintf(stderr, "Got MAV_CMD_DO_SET_MODE packet, mode %d\n", packetbuf[6]);
			switch (packetbuf[6]) {		// switch on the mode setting
				case 220:	// MAV_MODE_AUTO_ARMED
				case 92:	// MAV_MODE_AUTO_DISARMED
					fprintf(stderr, "Got MAV_MODE_AUTO command; turning on tracking\n");
					packetbuf[(packetlen-1)] += 1;	// stomp the crc so that this doesn't get interpretted farther down the line
					writeLEDs(1);
					(*trackFlag) = -1;
					break;
				case 0: 	// MAV_MODE_PREFLIGHT
                case 80:	// MAV_MODE_STABILIZE_DISARMED
                case 208: 	// MAV_MODE_STABILIZE_ARMED
                case 64:	// MAV_MODE_MANUAL_DISARMED
				case 192:	// MAV_MODE_MANUAL_ARMED
				case 88:	// MAV_MODE_GUIDED_DISARMED
				case 216:	// MAV_MODE_GUIDED_ARMED
					fprintf(stderr, "Got mode %d command; turning off tracking\n", packetbuf[6]);
					writeLEDs(0);
					(*trackFlag) = 0;
					break;
				default:
					break;
			}
		default:
			break;
	}
	
	return 0;
}

inline int getModPacket (FILE *mod, int16_t *commands) {
	float roll, pitch, yaw, throttle;
	int fields;
	
	// grab the data from the pseudo TTY
	fields = fscanf(mod, MODPKTSCANF, &roll, &pitch, &yaw, &throttle);		
	
	// bound the inputs
	roll = boundValueFloat(roll, 1.0, -1.0);	
	pitch = boundValueFloat(pitch, 1.0, -1.0);	
	yaw = boundValueFloat(yaw, 1.0, -1.0);	
	throttle = boundValueFloat(throttle, 1.0, -1.0);	
	
	// scale the inputs and write them to the command 
	commands[MODROLLOFFSET] = (int16_t)(roll * ROLLGAIN);				 
	commands[MODPITCHOFFSET] = (int16_t)(pitch * PITCHGAIN);
	commands[MODYAWOFFSET] = (int16_t)(yaw * YAWGAIN);
	commands[MODTHRTLOFFSET] = (int16_t)(throttle * THROTTLEGAIN);
	
	return fields;
}

inline float boundValueFloat (float val, float upper, float lower) {
	if (val > upper) val = upper;
	else if (val < lower) val = lower;
	return val;
}

inline uint16_t boundValueUInt16 (uint16_t val, uint16_t upper, uint16_t lower) {
	if (val > upper) val = upper;
	else if (val < lower) val = lower;
	return val;
}

int procRCOverridePacket (unsigned char *packetbuf, int packetlen, int16_t *commands) {
	uint16_t roll, pitch, yaw, throttle;		// these are the values we're modifying
	uint16_t crc;								// new crc value
	
	roll = (uint16_t)((uint16_t)(packetbuf[(6+MODROLLOFFSET*2)]<<8) + 	// calculate incoming roll command
		(packetbuf[(6+1+MODROLLOFFSET*2)]));
	pitch = (uint16_t)((uint16_t)(packetbuf[(6+MODPITCHOFFSET*2)]<<8) + // calculate incoming pitch command
		(packetbuf[(6+1+MODPITCHOFFSET*2)]));
	yaw = (uint16_t)((uint16_t)(packetbuf[(6+MODYAWOFFSET*2)]<<8) + 	// calculate incoming yaw command
		(packetbuf[(6+1+MODYAWOFFSET*2)]));
	throttle = (uint16_t)((uint16_t)(packetbuf[(6+MODTHRTLOFFSET*2)]<<8) + 	// calculate incoming throttle command
		(packetbuf[(6+1+MODTHRTLOFFSET*2)]));
	
	fprintf(stderr, "\t\t\tRoll\tPitch\tYaw\tThrottle\n");
	fprintf(stderr, "Incoming\t%d\t%d\t%d\t%d\n", roll, pitch, yaw, throttle);
	fprintf(stderr, "Mod\t\t%d\t%d\t%d\t%d\n", 
		commands[MODROLLOFFSET], 
		commands[MODPITCHOFFSET],
		commands[MODYAWOFFSET], 
		commands[MODTHRTLOFFSET]);
		
	// calculate the output values	
	roll += commands[MODROLLOFFSET];
	pitch += commands[MODPITCHOFFSET];
	yaw += commands[MODYAWOFFSET];
	throttle += commands[MODTHRTLOFFSET];
	
	// bound the output values
	roll 		= boundValueUInt16(roll, MAXRCCMD, MINRCCMD);
	pitch 		= boundValueUInt16(pitch, MAXRCCMD, MINRCCMD);
	yaw 		= boundValueUInt16(yaw, MAXRCCMD, MINRCCMD);
	throttle 	= boundValueUInt16(throttle, MAXRCCMD, MINRCCMD);
	
	// print them out so we can check them
	fprintf(stderr, "Output\t%d\t%d\t%d\t%d\n", roll, pitch, yaw, throttle);	
	
	// write them out to the packet buffer
	packetbuf[(6+MODROLLOFFSET*2)] = (char)(roll >> 8);
	packetbuf[(7+MODROLLOFFSET*2)] = (char)(roll & 0xff);
	packetbuf[(6+MODPITCHOFFSET*2)] = (char)(pitch >> 8);
	packetbuf[(7+MODPITCHOFFSET*2)] = (char)(pitch & 0xff);
	packetbuf[(6+MODYAWOFFSET*2)] = (char)(yaw >> 8);
	packetbuf[(7+MODYAWOFFSET*2)] = (char)(yaw & 0xff);
	packetbuf[(6+MODTHRTLOFFSET*2)] = (char)(throttle >> 8);
	packetbuf[(7+MODTHRTLOFFSET*2)] = (char)(throttle & 0xff);
	
	// rewrite the CRC
	crc = crc_calculate((uint8_t *)(packetbuf+1), (packetlen-3));
	packetbuf[(packetlen-2)] = (unsigned char)(crc & 0xff);
	packetbuf[(packetlen-1)] = (unsigned char)((crc>>8) & 0xff);
	
	return 0;
	
}

int writeLEDs(int val) {
	static int flag = 0;
	static FILE *led0, *led1, *led2, *led3;
	
	if (!flag) {
		led0 = fopen("/sys/class/gpio/gpio32", "w");
		led1 = fopen("/sys/class/gpio/gpio26", "w");
		led2 = fopen("/sys/class/gpio/gpio127", "w");
		led3 = fopen("/sys/class/gpio/gpio48", "w");
	}
	fprintf(led0, "%d", val);
	fprintf(led1, "%d", val);
	fprintf(led2, "%d", val);
	fprintf(led3, "%d", val);
	
	return errno;
}
