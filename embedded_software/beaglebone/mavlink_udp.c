/*******************************************************************************
 Copyright (C) 2010  Bryan Godbolt godbolt ( a t ) ualberta.ca
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 ****************************************************************************/
/*
 This program sends some data to qgroundcontrol using the mavlink protocol.  The sent packets
 cause qgroundcontrol to respond with heartbeats.  Any settings or custom commands sent from
 qgroundcontrol are printed by this program along with the heartbeats.
 
 
 I compiled this program sucessfully on Ubuntu 10.04 with the following command
 
 gcc -I ../../pixhawk/mavlink/include -o udp-server udp-server-test.c
 
 the rt library is needed for the clock_gettime on linux
 */
#include "includes.h"

/*****************************************************************************/
/* System defines
/*****************************************************************************/
#define BUFFER_LENGTH 2041 // minimum buffer size that can be used with qnx (I don't know why)

#define LOG_FILE_NAME "./mavlink_udp.log"

// Identification for our system
#define SYSTEM_ID          2
#define MAV_VEHICLE_TYPE   MAV_TYPE_SURFACE_BOAT
#define MAV_AUTOPILOT_TYPE MAV_AUTOPILOT_GENERIC
#define SENSORS (MAV_SYS_STATUS_SENSOR_3D_GYRO|MAV_SYS_STATUS_SENSOR_3D_ACCEL|MAV_SYS_STATUS_SENSOR_3D_MAG|MAV_SYS_STATUS_SENSOR_GPS|MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS)

/*****************************************************************************/
/* Function prototypes
/*****************************************************************************/
uint64_t microsSinceEpoch(void);
void listParams(void);
void parsePacket(void);

/*****************************************************************************/
/* Global vars
/*****************************************************************************/
char help[] = "--help";
char target_ip[100];

int sock;
struct sockaddr_in gcAddr;
struct sockaddr_in locAddr;
uint8_t buf[BUFFER_LENGTH];
ssize_t recsize;
socklen_t fromlen;
int bytes_sent;

FILE *logFile;

mavlink_message_t msg;
mavlink_status_t status;
uint16_t len;

int i;
int functionReturnValue;
unsigned int temp = 0;

uint64_t timeOfLastHeartbeat;
position_t position = {0, 0, 0, 0, 0, 0, 0};

int main(int argc, char* argv[]) {
    // Check if --help flag was used
    if (argc == 2) {
        functionReturnValue = strcmp(argv[1], help);
        if (functionReturnValue == 0) {
            printf("\n");
            printf("\tUsage:\n\n");
            printf("\t");
            printf("%s", argv[0]);
            printf(" <ip address of QGroundControl>\n");
            printf("\tDefault for localhost: udp-server 127.0.0.1\n\n");
            exit(EXIT_FAILURE);
        } else {}
    } else {}

    // Change the target ip if parameter was given
    strcpy(target_ip, "127.0.0.1");
    if (argc == 2) {
        strcpy(target_ip, argv[1]);
    } else {}

    // Open network socket
    memset(&locAddr, 0, sizeof(locAddr));
    locAddr.sin_family = AF_INET;
    locAddr.sin_addr.s_addr = INADDR_ANY;
    locAddr.sin_port = htons(14551);

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // Bind the socket to port 14551 - necessary to receive packets from qgroundcontrol
    functionReturnValue = bind(sock,                         // Socket identifier
                               (struct sockaddr *) &locAddr, // Bind location & address
                               sizeof(struct sockaddr));     // Etc.
    if (functionReturnValue == -1) { // Warn us if bind fails
        perror("error bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    } else {}

    // Attempt to make socket non blocking
    functionReturnValue = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (functionReturnValue < 0) {
        fprintf(stderr, "error setting nonblocking: %s\n", strerror(errno));
        close(sock);
        exit(EXIT_FAILURE);
    } else {}

    memset(&gcAddr, 0, sizeof(gcAddr));
    gcAddr.sin_family = AF_INET;
    gcAddr.sin_addr.s_addr = inet_addr(target_ip);
    gcAddr.sin_port = htons(14550);

    timeOfLastHeartbeat = microsSinceEpoch(); // Prevent failsafe on first loop

    // Main loop
    for (;;) {
        logFile = fopen(LOG_FILE_NAME, "a");

        // Send Heartbeat
        mavlink_msg_heartbeat_pack(SYSTEM_ID,                  // System ID
                                   MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                   &msg,                       // Message buffer
                                   MAV_VEHICLE_TYPE,           // MAV vehicle type
                                   MAV_AUTOPILOT_TYPE,         // Autopilot type
                                   MAV_MODE_GUIDED_ARMED,      // System mode
                                   0,                          // Custom mode (empty)
                                   MAV_STATE_ACTIVE);          // System status

        len = mavlink_msg_to_send_buffer(buf, &msg);

        bytes_sent = sendto(sock,                         // Outgoing device
                            buf,                          // Buffer
                            len,                          // Buffer size
                            0,                            // Flags
                            (struct sockaddr*)&gcAddr,    // Address
                            sizeof(struct sockaddr_in));  // Address length

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d heartbeat", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d heartbeat", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

        // Send Status
        mavlink_msg_sys_status_pack(SYSTEM_ID,                  // System ID
                                    MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                    &msg,                       // Message buffer
                                    SENSORS,                    // Sensors present
                                    SENSORS,                    // Sensors enabled
                                    SENSORS,                    // Sensor health
                                    500,                        // System load
                                    12000,                      // Batt voltage
                                    -1,                         // Batt current
                                    -1,                         // Batt remaining
                                    0,                          // Comm drop percentage
                                    0,                          // Comm errors
                                    0,                          // Custom error 1
                                    0,                          // Custom error 2
                                    0,                          // Custom error 3
                                    0);                         // Custom error 4

        len = mavlink_msg_to_send_buffer(buf, &msg);

        bytes_sent = sendto(sock,                        // Outgoing device
                            buf,                         // Buffer
                            len,                         // Buffer size
                            0,                           // Flags
                            (struct sockaddr*) &gcAddr,  // Address
                            sizeof(struct sockaddr_in)); // Address length

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d status", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d status", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

        // Send GPS Position
        mavlink_msg_global_position_int_pack(SYSTEM_ID,          // System ID
                                             MAV_COMP_ID_GPS,    // Component ID
                                             &msg,               // Message buffer
                                             microsSinceEpoch(), // Current time
                                             position.lat,       // Lat
                                             position.lon,       // Long
                                             position.alt,       // Alt
                                             position.alt,       // Relative alt (assume same)
                                             position.vx,        // X vel
                                             position.vy,        // Y vel
                                             position.vz,        // Z vel
                                             position.hdg);      // Heading

        len = mavlink_msg_to_send_buffer(buf, &msg);

        bytes_sent = sendto(sock,                        // Outgoing device
                            buf,                         // Buffer
                            len,                         // Buffer size
                            0,                           // Flags
                            (struct sockaddr*) &gcAddr,  // Address
                            sizeof(struct sockaddr_in)); // Address length

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d GPS", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d GPS", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

        // Update GPS position
        position.lat++;

        // Update compass

        // Receive incoming packets from network interface
        memset(buf, 0, BUFFER_LENGTH);
        recsize = recvfrom(sock,                         // Socket device
                           (void *) buf,                 // Receive buffer
                           BUFFER_LENGTH,                // Length of buffer
                           0,                            // Extra settings (none)
                           (struct sockaddr *) &gcAddr, // Receive from address
                           &fromlen);                    // Receive address length
        if (recsize > 0) {
            // Something received - print out all bytes and parse packet
            //printf("Bytes Received: %d\nDatagram: ", (int)recsize);
            for (i = 0; i < recsize; ++i) {
                temp = buf[i];
                //printf("%02x ", (unsigned char)temp);
                functionReturnValue = mavlink_parse_char(MAVLINK_COMM_0, // Channel
                                                         buf[i],         // Char to parse
                                                         &msg,           // Message buffer
                                                         &status);       // Message status
                if (functionReturnValue) {
                    // Packet received
                    printf("\nReceived packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d ", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
                    fprintf(logFile, "\nReceived packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d ", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
                    parsePacket();
                } else {}
            }
        } else {}
        memset(buf, 0, BUFFER_LENGTH);

        // Watchdog return, if out of contact for 10 seconds, beach self
        if ((microsSinceEpoch()-timeOfLastHeartbeat) > 10000000) {
            printf("\n*** FAILSAFE ***");
            fprintf(logFile, "\n*** FAILSAFE ***");
            // Head for beach
        } else {}

        fclose(logFile);
        sleep(1); // Sleep one second
    }
}

uint64_t microsSinceEpoch(void) {

    struct timeval tv;

    uint64_t micros = 0;

    gettimeofday(&tv, NULL);
    micros =  ((uint64_t)tv.tv_sec) * 1000000 + tv.tv_usec;

    return micros;
}

void parsePacket(void) {
    //printf("\nWe got a packet!\n");
    switch (msg.msgid) { // Decide what to do with packet
    case MAVLINK_MSG_ID_HEARTBEAT: // Record time of last heartbeat for watchdog
        printf("heartbeat");
        fprintf(logFile, "heartbeat");
        timeOfLastHeartbeat = microsSinceEpoch();
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:

        break;
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        printf("param request");
        fprintf(logFile, "param request");
        listParams();
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM:
        printf("mission item");
        fprintf(logFile, "mission item");
        break;
    default:
        printf("other packet type");
        fprintf(logFile, "other packet type");
        break;
    }
}

void listParams(void) {
    mavlink_msg_param_value_pack(SYSTEM_ID,                  // System ID
                                 MAV_COMP_ID_SYSTEM_CONTROL, // Component
                                 &msg,                       // Message buffer
                                 "NOPARAMS",                 // Param name
                                 0,                          // Param value (dummy)
                                 MAVLINK_TYPE_FLOAT,         // Param type
                                 1,                          // Total params on system
                                 0);                         // Current param index

    len = mavlink_msg_to_send_buffer(buf, &msg);

    bytes_sent = sendto(sock,                         // Outgoing device
                        buf,                          // Buffer
                        len,                          // Buffer size
                        0,                            // Flags
                        (struct sockaddr*)&gcAddr,    // Address
                        sizeof(struct sockaddr_in));  // Address length
}
