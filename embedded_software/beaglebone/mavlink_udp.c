/*****************************************************************************/
/** @file mavlink_udp.c
* @brief Hackerboat Mavlink Control
* @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
* @author Bryan Godbolt godbolt ( a t ) ualberta.ca
* @author Wim Lewis wiml ( a t ) omnigroup.com
* @date 2014
*
* GNU General Public License blah blah blah
*
* This program sends some data to qgroundcontrol using the mavlink protocol.
*
*/
/*****************************************************************************/

#include "includes.h"
#include <err.h>

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
#define SEC_UNTIL_PANIC 10 // Seconds without heartbeat until failsafe mode
#define NUM_MISSION_ITEMS 256 // Number of mission items (waypoints) we can store

#define ACCEPTABLE_DISTANCE_FROM_WAYPOINT 100 // Meters
#define EAST 90

// Special values for checkDeadline
#define TV_DEADLINE_UNSET ((struct timeval){0, -1})
#define TV_DEADLINE_IMMEDIATE ((struct timeval){0, -2})


/*****************************************************************************/
/* Function prototypes
/*****************************************************************************/
uint64_t microsSinceEpoch(void);
void parseInputParams(int argc, char* argv[]);
void openNetworkSocket(void);
void sendMavlinkPacketOverNetwork(void);
void listParams(void);
void oncePerSecond(void);
void processMavlinkActivity(void);
void parsePacket(void);
void setMode(void);
void sendNumMissionItems(void);
void sendMissionItem(void);
void receiveMissionCount(void);
void receiveMissionItem(void);
void clearAllMissionItems(void);
void parseCommand(void);
void reachedTargetWaypoint(void);
float getDistanceToWaypoint(void);
float getAngleToWaypoint(void);
void readGpsMessages(void);
void processGpsMessage(char *m, int msglen);
void processGPRMC(const char **fields, int fieldCount);
void openUarts(void);
void sendPacketToLowLevel(void);
void readLowLevelVoltageLevel(void);
void getUartDebug(void);
float stringToFloat(position_string_t position);
int fromHex2(char hh, char ll);
int readFromDevice(int fd, char buf[], int buflen);
struct timeval timevalAddf(struct timeval tv, double increment);
int checkDeadline(struct timeval now, struct timeval deadline, struct timeval *timeout);
struct timeval nextDeadline(struct timeval deadline, struct timeval tv, double increment, double noLessThan);

/*****************************************************************************/
/* Global vars
/*****************************************************************************/

// Param buffers
char target_ip[100];
char gps_serial_device[100];
char low_level_serial_device[100];
char low_level_debug_device[100];

// Network socket vars
int sock;
struct sockaddr_in gcAddr;   // Ground Control address (our mavlink peer)

// UART vars
int lowLevelControlFD;
int lowLevelDebugFD;
int gpsModuleFD;

// GPS reception buffer
// we accumulate bytes from the GPS UART here until we have a full line
#define GPS_BUFFER_SIZE   512
char gpsBuffer[GPS_BUFFER_SIZE];
int gpsBufferUsed;

// Logfile vars
FILE *logFile;
time_t currentTime;

// Mavlink vars
mavlink_message_t msg;
mavlink_status_t status;

struct timeval timeOfLastHeartbeat;
position_t position = {0, 0, 0, 0, 0, 0, 0, 0};
position_t target;
uint8_t targetWaypoint;
mavlink_command_long_t currentCommand;
uint16_t voltageLevel;
uint8_t goodToGo;

MAV_MODE currentMavMode;
MAV_STATE currentMavState;

mavlink_mission_item_t missionItems[NUM_MISSION_ITEMS];
uint16_t numMissionItems;
uint16_t currentlyInterogatedMissionItem;
uint16_t currentlyActiveMissionItem;


/*****************************************************************************/
/* Main                                                                      */
/*****************************************************************************/
int main(int argc, char* argv[]) {
    struct timeval nextHeartbeatTx;
    
    parseInputParams(argc, argv); // Parse input params

    openNetworkSocket(); // Open network socket

    openUarts(); // Open serial ports to GPS and low-level controller

    logFile = fopen(LOG_FILE_NAME, "a");

    timeOfLastHeartbeat = TV_DEADLINE_UNSET; // Prevent failsafe on first loop
    nextHeartbeatTx = TV_DEADLINE_IMMEDIATE;
    currentMavState = MAV_STATE_BOOT; // Put us on standby state
    currentMavMode = MAV_MODE_MANUAL_DISARMED; // Put us in manual mode

    // Set mission item index to 0
    numMissionItems = 0;
    currentlyActiveMissionItem = 0;
    targetWaypoint = 0;
    voltageLevel = 0;
    goodToGo = 0;

    // Main loop
    for (;;) {
        int fdmax;
        fd_set rfds;
        struct timeval now_time, maxWait;

        /* Figure out how long until one of our timed events happens (either
         * our oncePerSecond() processing or our link-dropped panic)
         */
        gettimeofday(&now_time, NULL); /* TODO: use CLOCK_MONOTONIC instead? */
        maxWait = (struct timeval){ .tv_sec = 1, .tv_usec = 0 };
        // printf("checking nextTx (%ld/%ld) against now (%ld/%ld)\n",
        //   nextHeartbeatTx.tv_sec, nextHeartbeatTx.tv_usec,
        //   now_time.tv_sec, now_time.tv_usec);
        if (checkDeadline(now_time, nextHeartbeatTx, &maxWait)) {
            oncePerSecond();
            nextHeartbeatTx = nextDeadline(nextHeartbeatTx, now_time, 1.0, 0.25);
            printf("advancing nextTx to (%ld/%ld)\n",
                   nextHeartbeatTx.tv_sec, nextHeartbeatTx.tv_usec);
            continue;
        }
#if 0
        // TODO: We want to panic if it's been too long since we
        // received a heartbeat packet, *but* we need to not re-panic
        // each time through the loop once we're in a panic state. We
        // should probably depend on currentMavMode, currentMavState,
        // or something in this test and not panic if we're in BOOT
        // etc. or already in return-to-base mode.
        if (... && checkDeadline(now_time, timevalAddf(timeOfLastHeartbeat, SEC_UNTIL_PANIC), &maxWait)) {
            // Watchdog return, if out of contact for 10 seconds
            linkInactivityPanic();
        }
#endif

        FD_ZERO(&rfds);
        fdmax = 0;
#define SET_ONE(f)   do { if (f >= 0) { FD_SET(f, &rfds); if (f >= fdmax) fdmax = f + 1; } }while(0)
        SET_ONE(sock);
        SET_ONE(lowLevelDebugFD);
        SET_ONE(lowLevelControlFD);
        SET_ONE(gpsModuleFD);
#undef SET_ONE

        int c = select(fdmax, &rfds, NULL, NULL, &maxWait);
        if (c < 0) {
            /* The select() call failed. Why? */
            if (errno != EAGAIN && errno != EINTR) {
                /* An actual failure */
                err(EXIT_FAILURE, "select");
            } else {
                /* A signal or something. Just recalculate our timers and go again. */
                continue;
            }
        }

        // Receive incoming packets from network interface
        if (FD_ISSET(sock, &rfds)) {
            processMavlinkActivity();
        } else {}

        // Read voltage level from low level device
        if (FD_ISSET(lowLevelControlFD, &rfds)) {
            readLowLevelVoltageLevel();
        } else {}

        // Read data from the GPS
        if (FD_ISSET(gpsModuleFD, &rfds)) {
            readGpsMessages();
        } else {}

        // Read log debug messages from the low level controller
        if (FD_ISSET(lowLevelDebugFD, &rfds)) {
            getUartDebug();
        } else {}

        // Flush buffers to force output
        fflush(NULL);
    }
}

/** Once per second tasks
*
* Sends specific mavlink packets once per second:
*
* - Heartbeat
* - Status
* - GPS position
* - Current target waypoint
* - Attitude control
*
* Transmissions are sent to stdout, logged to file and sent over network
* interface.
*/
void oncePerSecond(void) {
    currentTime = time(0);
    printf("\n%s", ctime(&currentTime));
    fprintf(logFile, "\n%s", ctime(&currentTime));

    // Send Heartbeat
    mavlink_msg_heartbeat_pack(SYSTEM_ID,                  // System ID
                               MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                               &msg,                       // Message buffer
                               MAV_VEHICLE_TYPE,           // MAV vehicle type
                               MAV_AUTOPILOT_TYPE,         // Autopilot type
                               currentMavMode,             // System mode
                               0,                          // Custom mode (empty)
                               currentMavState);           // System status

    sendMavlinkPacketOverNetwork();

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
                                voltageLevel,               // Batt voltage
                                -1,                         // Batt current
                                -1,                         // Batt remaining
                                0,                          // Comm drop percentage
                                0,                          // Comm errors
                                0,                          // Custom error 1
                                0,                          // Custom error 2
                                0,                          // Custom error 3
                                0);                         // Custom error 4

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d status", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d status", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

    // Send GPS Position
    mavlink_msg_global_position_int_pack(SYSTEM_ID,                             // System ID
                                         MAV_COMP_ID_GPS,                       // Component ID
                                         &msg,                                  // Message buffer
                                         microsSinceEpoch(),                    // Current time
                                         ((int32_t) (position.lat * 10000000)), // Lat
                                         ((int32_t) (position.lon * 10000000)), // Long
                                         position.alt,                          // Alt
                                         position.alt,                          // Relative alt (assume same)
                                         position.vx,                           // X vel
                                         position.vy,                           // Y vel
                                         position.vz,                           // Z vel
                                         position.hdg);                         // Heading

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d GPS", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d GPS", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

    // Send current waypoint if in auto mode
    if (currentMavMode == MAV_MODE_AUTO_ARMED) {
        mavlink_msg_mission_current_pack(SYSTEM_ID,
                                         MAV_COMP_ID_SYSTEM_CONTROL,
                                         &msg,
                                         targetWaypoint);

        sendMavlinkPacketOverNetwork();

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d current mission", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d current mission", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    } else {}

    if ((currentMavMode == MAV_MODE_AUTO_ARMED) || (currentMavMode == MAV_MODE_AUTO_DISARMED)) {
        float distanceToWaypoint = getDistanceToWaypoint();
        if (distanceToWaypoint <= ACCEPTABLE_DISTANCE_FROM_WAYPOINT) {
            reachedTargetWaypoint();
        } else {
            if (goodToGo) {
                float angleToWaypoint = getAngleToWaypoint();
                mavlink_msg_attitude_control_pack(SYSTEM_ID,
                                                  MAV_COMP_ID_PATHPLANNER,
                                                  &msg,
                                                  SYSTEM_ID,
                                                  0,
                                                  0,
                                                  angleToWaypoint,
                                                  1,
                                                  0,
                                                  0,
                                                  0,
                                                  0);

                sendMavlinkPacketOverNetwork();

                // Transmitted packet
                printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d attitude command", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
                fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d attitude command", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
            } else {}
        }
    } else if ((currentMavMode == MAV_MODE_MANUAL_ARMED) || (currentMavMode == MAV_MODE_MANUAL_ARMED)) {
        float distanceToWaypoint = getDistanceToWaypoint();
        if (distanceToWaypoint <= ACCEPTABLE_DISTANCE_FROM_WAYPOINT) {
            reachedTargetWaypoint();
        } else {}
    } else {}
    // Update compass
}

/** Send in emergency to head back to shore
*
* When triggered, linkInactivityPanic will set goodToGo false and command low
* level control to head east.
*/
void linkInactivityPanic(void) {
    goodToGo = 0;
    
    mavlink_msg_attitude_control_pack(SYSTEM_ID,
                                      MAV_COMP_ID_PATHPLANNER,
                                      &msg,
                                      SYSTEM_ID,
                                      0,
                                      0,
                                      ((float) EAST),
                                      1,
                                      0,
                                      0,
                                      0,
                                      0);
    
    sendMavlinkPacketOverNetwork();
    
    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d failsafe to shore", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d failsafe to shore", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
}

/** Parses incoming mavlink packets from UDP interface
*
* When packet is receive successfully function calls parsePacket. Each received
* packet is also retransmitted on the UART interface to low level control.
*/
void processMavlinkActivity(void) {
    uint8_t buf[BUFFER_LENGTH];
    ssize_t recsize;
    socklen_t fromLen;
    int i;

    memset(buf, 0, BUFFER_LENGTH);
    recsize = recvfrom(sock,                         // Socket device
                       (void *) buf,                 // Receive buffer
                       BUFFER_LENGTH,                // Length of buffer
                       0,                            // Extra settings (none)
                       (struct sockaddr *) &gcAddr, // Receive from address
                       &fromLen);                    // Receive address length
    if (recsize > 0) {
        // Something received - print out all bytes and parse packet
        //printf("Bytes Received: %d\nDatagram: ", (int)recsize);
        for (i = 0; i < recsize; ++i) {
            uint8_t receivedMessage = mavlink_parse_char(MAVLINK_COMM_0, // Channel
                                                         buf[i],         // Char to parse
                                                         &msg,           // Message buffer
                                                         &status);       // Message status
            if (receivedMessage) {
                // Packet received
                printf("\nReceived packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d ", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
                fprintf(logFile, "\nReceived packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d ", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);

                sendPacketToLowLevel();
                parsePacket();
            } else {}
        }
    } else {}
}

/** Returns number of microseconds since epoch
*
* @return Number of microseconds since epoch.
*/
uint64_t microsSinceEpoch(void) {

    struct timeval tv;

    uint64_t micros = 0;

    gettimeofday(&tv, NULL);
    micros =  ((uint64_t)tv.tv_sec) * 1000000 + tv.tv_usec;

    return micros;
}

/** Parses program input parameters
*
* An argument count of five is expected:
*
* - 0. Always program name
* - 1. Target ipv4 of ground control station
* - 2. tty of gps device
* - 3. tty of low level control device
* - 4. tty of debug interface
*
* If incorrect number of params are passed, program will display help msg and
* exit.
*
* @param argc Argument count
* @param argv Char array of arguments entered
*/
void parseInputParams(int argc, char* argv[]) {
    strcpy(target_ip, "127.0.0.1"); // Set default ip
    if (argc == 5) {
        strcpy(target_ip, argv[1]); // Change the target ip if parameter was given
        strcpy(gps_serial_device, argv[2]);
        strcpy(low_level_serial_device, argv[3]);
        strcpy(low_level_debug_device, argv[4]);
    } else {
        printf("\n");
        printf("\tUsage:\n\n");
        printf("\t");
        printf("%s", argv[0]);
        printf(" <host ip> <GPS serial device> <low level serial device>\n");
        printf("\tDefault for localhost: udp-server 127.0.0.1 /dev/tty1 /dev/tty2\n\n");
        exit(EXIT_FAILURE);
    }
}

/** Opens UDP network socket
*
*/
void openNetworkSocket(void) {
    int rc;
    struct sockaddr_in locAddr;
    memset(&locAddr, 0, sizeof(locAddr));
    locAddr.sin_family = AF_INET;
    locAddr.sin_addr.s_addr = INADDR_ANY;
    locAddr.sin_port = htons(14551);

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
        err(EXIT_FAILURE, "socket");
    // Bind the socket to port 14551 - necessary to receive packets from qgroundcontrol
    rc = bind(sock,                         // Socket identifier
              (struct sockaddr *) &locAddr, // Bind location & address
              sizeof(struct sockaddr));     // Etc.
    if (rc == -1) { // Warn us if bind fails
        perror("error bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    } else {}

    // Attempt to make socket non blocking
    rc = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        fprintf(stderr, "error setting nonblocking: %s\n", strerror(errno));
        close(sock);
        exit(EXIT_FAILURE);
    } else {}

    /* Initial mavlink peer address */
    memset(&gcAddr, 0, sizeof(gcAddr));
    gcAddr.sin_family = AF_INET;
    gcAddr.sin_addr.s_addr = inet_addr(target_ip);
    gcAddr.sin_port = htons(14550);
}

/** Sends a mavlink packet over the network interface
*
* Function assumes global buffer \c msg has been packed with message content.
* \c msg is not modified by this function.
*/
void sendMavlinkPacketOverNetwork(void) {
    uint8_t buf[BUFFER_LENGTH];
    int bytes_sent;
    uint16_t buffer_used;

    memset(buf, 0, sizeof(buf));

    buffer_used = mavlink_msg_to_send_buffer(buf, &msg);
    
    bytes_sent = sendto(sock,                         // Outgoing device
                        buf,                          // Buffer
                        buffer_used,                  // Buffer size
                        0,                            // Flags
                        (struct sockaddr*)&gcAddr,    // Address
                        sizeof(struct sockaddr_in));  // Address length
}

/** Parses received mavlink packet calls appropriate function
*
* Add new packet types we need to respond to in this switch statement.
* Received functions are logged to stdout and file.
* Heartbeat packets cause time of day to be updated to prevent watchdog panic.
*/
void parsePacket(void) {
    //printf("\nWe got a packet!\n");
    switch (msg.msgid) { // Decide what to do with packet
    case MAVLINK_MSG_ID_HEARTBEAT: // Record time of last heartbeat for watchdog
        printf("heartbeat");
        fprintf(logFile, "heartbeat");
        gettimeofday(&timeOfLastHeartbeat, NULL);
        break;
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        printf("param request");
        fprintf(logFile, "param request");
        listParams();
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        printf("mission list request");
        fprintf(logFile, "mission list request");
        sendNumMissionItems();
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        printf("mission item request %d", mavlink_msg_mission_request_get_seq(&msg));
        fprintf(logFile, "mission item request %d", mavlink_msg_mission_request_get_seq(&msg));
        sendMissionItem();
        break;
    case MAVLINK_MSG_ID_MISSION_ACK:
        printf("mission ack");
        fprintf(logFile, "mission ack");
        break;
    case MAVLINK_MSG_ID_MISSION_COUNT:
        printf("mission count %d", mavlink_msg_mission_count_get_count(&msg));
        fprintf(logFile, "mission count %d", mavlink_msg_mission_count_get_count(&msg));
        receiveMissionCount();
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM:
        printf("mission item %d", mavlink_msg_mission_item_get_seq(&msg));
        fprintf(logFile, "mission item %d", mavlink_msg_mission_item_get_seq(&msg));
        receiveMissionItem();
        break;
    case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
        printf("clear all mission items");
        fprintf(logFile, "clear all mission items");
        clearAllMissionItems();
        break;
    case MAVLINK_MSG_ID_SET_MODE:
        printf("set mode");
        fprintf(logFile, "set mode");
        setMode();
        break;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        printf("command");
        fprintf(logFile, "command");
        parseCommand();
        break;
    default:
        printf("other packet type");
        fprintf(logFile, "other packet type");
        break;
    }
}

/** Tells qgroundcontrol that we have no internal parameters
*
* Packet is transmitted and logged to stdout and file.
*/
void listParams(void) {
    mavlink_msg_param_value_pack(SYSTEM_ID,                  // System ID
                                 MAV_COMP_ID_SYSTEM_CONTROL, // Component
                                 &msg,                       // Message buffer
                                 "NOPARAMS",                 // Param name
                                 0,                          // Param value (dummy)
                                 MAVLINK_TYPE_FLOAT,         // Param type
                                 1,                          // Total params on system
                                 0);                         // Current param index

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d params list", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d params list", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
}

/** Sets currentMavMode to mode commanded by received mavlink packet
*
*/
void setMode(void) {
    currentMavMode = mavlink_msg_set_mode_get_base_mode(&msg);
}

/** Responds to request for number of mission items
*
* Packet is transmitted and logged to stdout and file.
*/
void sendNumMissionItems(void) {
    mavlink_msg_mission_count_pack(SYSTEM_ID,                  // System ID
                                   MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                   &msg,                       // Message buffer
                                   msg.sysid,                  // SysID of system making request
                                   msg.compid,                 // CompID of system making request
                                   numMissionItems);           // Number of currently programmed mission items

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item count %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, numMissionItems);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item count %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, numMissionItems);
}

/** Responds to request for specific mission item
*
* Packet is transmitted and logged to stdout and file.
* If requested mission item exceeds range of stored mission items, function
* responds with out of memory error.
*/
void sendMissionItem(void) {
    currentlyInterogatedMissionItem = mavlink_msg_mission_request_get_seq(&msg);

    if (currentlyInterogatedMissionItem < numMissionItems) {
        mavlink_msg_mission_item_encode(SYSTEM_ID,                                       // System ID
                                        MAV_COMP_ID_SYSTEM_CONTROL,                      // Component ID
                                        &msg,                                            // Message buffer
                                        &missionItems[currentlyInterogatedMissionItem]); // Mission item structure from array of mission items

        sendMavlinkPacketOverNetwork();

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, currentlyInterogatedMissionItem);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, currentlyInterogatedMissionItem);
    } else {
        printf("\nWe don't have that mission item");
        fprintf(logFile, "\nWe don't have that mission item");
        mavlink_msg_mission_ack_pack(SYSTEM_ID,                  // System ID
                                     MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                     &msg,                       // Message buffer
                                     msg.sysid,                  // SysID of requesting system
                                     msg.compid,                 // CompID of requesting system
                                     MAV_MISSION_ERROR);         // We must be out of space

        sendMavlinkPacketOverNetwork();
    }
}

/** Prepares to receive mission items from qgroundcontrol
*
* We learn how many mission items to expect from qgroundcontrol and request
* the first one. Packet is transmitted and logged to stdout and file.
*/
void receiveMissionCount(void) {
    currentlyInterogatedMissionItem = 0;
    numMissionItems = mavlink_msg_mission_count_get_count(&msg);

    mavlink_msg_mission_request_pack(SYSTEM_ID,                        // System ID
                                     MAV_COMP_ID_SYSTEM_CONTROL,       // Component ID
                                     &msg,                             // Message buffer
                                     msg.sysid,                        // SysID of requesting system
                                     msg.compid,                       // CompID of requesting system
                                     currentlyInterogatedMissionItem); // ID of mission item requested

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d request mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, currentlyInterogatedMissionItem);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d request mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, currentlyInterogatedMissionItem);
}

/** qgroundcontrol sends us a mission item to store
*
* If we receive a mission we requested we store it. If this is the final
* mission item we need we send an ack packet. If received packet exceeds number
* of packets we were requesting, we send a \c MAV_MISSION_NO_SPACE error.
* All packet transmissions are logged to stdout and file.
*/
void receiveMissionItem(void) {
    currentlyInterogatedMissionItem = mavlink_msg_mission_item_get_seq(&msg);

    if (currentlyInterogatedMissionItem < (numMissionItems-1)) {
        mavlink_msg_mission_item_decode(&msg, &missionItems[currentlyInterogatedMissionItem]);
        missionItems[currentlyInterogatedMissionItem].target_system = msg.sysid;

        mavlink_msg_mission_request_pack(SYSTEM_ID,                            // System ID
                                         MAV_COMP_ID_SYSTEM_CONTROL,           // Component ID
                                         &msg,                                 // Message buffer
                                         msg.sysid,                            // SysID of requesting system
                                         msg.compid,                           // CompID of requesting system
                                         (currentlyInterogatedMissionItem+1)); // ID of mission item requested

        sendMavlinkPacketOverNetwork();

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d request mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, (currentlyInterogatedMissionItem+1));
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d request mission item %d", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid, (currentlyInterogatedMissionItem+1));
    } else if (currentlyInterogatedMissionItem == (numMissionItems-1)) {
        mavlink_msg_mission_item_decode(&msg, &missionItems[currentlyInterogatedMissionItem]);
        missionItems[currentlyInterogatedMissionItem].target_system = msg.sysid;

        mavlink_msg_mission_ack_pack(SYSTEM_ID,                  // System ID
                                     MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                     &msg,                       // Message buffer
                                     msg.sysid,                  // SysID of requesting system
                                     msg.compid,                 // CompID of requesting system
                                     MAV_MISSION_ACCEPTED);      // Accepting waypoint list

        sendMavlinkPacketOverNetwork();

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item list ack", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item list ack", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    } else {
        printf("\nCant store that many mission items");
        fprintf(logFile, "\nCant store that many mission items");
        mavlink_msg_mission_ack_pack(SYSTEM_ID,                  // System ID
                                     MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                     &msg,                       // Message buffer
                                     msg.sysid,                  // SysID of requesting system
                                     msg.compid,                 // CompID of requesting system
                                     MAV_MISSION_NO_SPACE);      // We must be out of space

        sendMavlinkPacketOverNetwork();

    }
}

/** Erases all stored mission items
*
* Sets currently stored number of mission items to 0 and sends ack packet.
* Packet transmission is logged to stdout and file.
*/
void clearAllMissionItems(void) {
    numMissionItems = 0;

    mavlink_msg_mission_ack_pack(SYSTEM_ID,                  // System ID
                                 MAV_COMP_ID_SYSTEM_CONTROL, // Component ID
                                 &msg,                       // Message buffer
                                 msg.sysid,                  // SysID of requesting system
                                 msg.compid,                 // CompID of requesting system
                                 MAV_MISSION_ACCEPTED);      // We must be out of space

    sendMavlinkPacketOverNetwork();

    // Transmitted packet
    printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item list ack", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d mission item list ack", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
}

/** Parses system commands send by qgroundcontrol
*
* Pressing the start/stop buttons in qgroundcontrol will send command packets.
* These allow mission start/stop, return, etc. to be controlled.
* Add new commands we must respond to into this switch statement.
*/
void parseCommand(void) {
    mavlink_msg_command_long_decode(&msg, &currentCommand);

    if (currentCommand.target_system == SYSTEM_ID) {
        switch (currentCommand.command) {
        case MAV_CMD_NAV_WAYPOINT:
            currentMavMode = MAV_MODE_MANUAL_ARMED;
            target.lat = currentCommand.param5;
            target.lon = currentCommand.param6;
            goodToGo = 1;
            break;
        case MAV_CMD_NAV_RETURN_TO_LAUNCH:
            break;
        case MAV_CMD_NAV_TAKEOFF:
            goodToGo = 1;
            break;
        case MAV_CMD_MISSION_START:
            break;
        default:
            break;
        }
    } else {}
}

/** Behavior upon reaching target waypoint vicinity
*
* Waypoints include commands to control the boat's next course of action.
* If/else statements control if next target waypoint is the next in the
* sequential waypoint list, or a waypoint id included with the achieved
* waypoint. It is also possible for no waypoint to be designated, and the boat
* will stop.
*/
void reachedTargetWaypoint(void) {
    if (missionItems[currentlyActiveMissionItem].command == MAV_CMD_DO_JUMP) {
        currentlyActiveMissionItem = missionItems[currentlyActiveMissionItem].param1;
    } else if (missionItems[currentlyActiveMissionItem].autocontinue) {
        currentlyActiveMissionItem++;
    } else if (missionItems[currentlyActiveMissionItem].autocontinue == 0) {
        currentMavMode = MAV_MODE_MANUAL_DISARMED;
        goodToGo = 0;

        mavlink_msg_attitude_control_pack(SYSTEM_ID,
                                          MAV_COMP_ID_PATHPLANNER,
                                          &msg,
                                          SYSTEM_ID,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0);

        sendMavlinkPacketOverNetwork();

        // Transmitted packet
        printf("\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d attitude command stop", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
        fprintf(logFile, "\nTransmitted packet: SEQ: %d, SYS: %d, COMP: %d, LEN: %d, MSG ID: %d attitude command stop", msg.seq, msg.sysid, msg.compid, msg.len, msg.msgid);
    } else {}
}

/** Finds current distance to target waypoint
*
* Calculates distance between current lat/lon and lat/lon of target waypoint
* using haversine method.
*
* Based off http://www.movable-type.co.uk/scripts/latlong.html
*
* @return great circle distance in meters to next waypoint
*/
float getDistanceToWaypoint(void) {
    float a;
    float c;
    float d;
    float targetLat;
    float targetLon;
    float currentLat;
    float currentLon;

    currentLat = (position.lat/(180/M_PI));
    currentLon = (position.lon/(180/M_PI));
    targetLat = (missionItems[currentlyActiveMissionItem].x/(180/M_PI));
    targetLon = (missionItems[currentlyActiveMissionItem].y/(180/M_PI));

    a = powf(sin((targetLat-currentLat)/2), 2)+cosf(currentLat)*cosf(targetLat)*powf(sinf((targetLon-targetLat)/2), 2);
    c = 2*atan2f(sqrtf(a), sqrtf(1-a));
    d = (c*6371)/1000;

    return (d);
}

/** Finds bearing to current target waypoint
*
* Calculates bearing to target waypoint from current lat/lon
*
* Based off http://www.movable-type.co.uk/scripts/latlong.html
*
* @return bearing in degrees towards current target waypoint
*/
float getAngleToWaypoint(void) {
    float angle;
    float targetLat;
    float targetLon;
    float currentLat;
    float currentLon;

    currentLat = (position.lat/(180/M_PI));
    currentLon = (position.lon/(180/M_PI));
    targetLat = (missionItems[currentlyActiveMissionItem].x/(180/M_PI));
    targetLon = (missionItems[currentlyActiveMissionItem].y/(180/M_PI));

    angle = atan2f((sinf(targetLon-currentLon)*cosf(targetLat)), (cosf(currentLat)*sinf(targetLat)-sinf(currentLat)*cosf(targetLat)*cosf(targetLon-currentLon)));
    angle *= (180/M_PI);

    return (angle);
}

/** Opens uart FDs
*
*/
void openUarts(void) {
    int lowLevelControlFlags;
    int lowLevelDebugFlags;
    int gpsModuleFlags;
    
    lowLevelControlFD = open(low_level_serial_device, O_RDWR);
    if (lowLevelControlFD < 0)
        err(EXIT_FAILURE, "%s: open", low_level_serial_device);
    lowLevelDebugFD = open(low_level_debug_device, O_RDONLY);
    if (lowLevelDebugFD < 0)
        err(EXIT_FAILURE, "%s: open", low_level_debug_device);
    gpsModuleFD = open(gps_serial_device, O_RDONLY);
    if (gpsModuleFD < 0)
        err(EXIT_FAILURE, "%s: open", gps_serial_device);
    
    lowLevelControlFlags = fcntl(lowLevelControlFD, F_GETFL, 0);
    fcntl(lowLevelControlFD, lowLevelControlFlags | O_NONBLOCK);
    
    lowLevelDebugFlags = fcntl(lowLevelDebugFD, F_GETFL, 0);
    fcntl(lowLevelDebugFD, lowLevelDebugFlags | O_NONBLOCK);
    
    gpsModuleFlags = fcntl(gpsModuleFD, F_GETFL, 0);
    fcntl(gpsModuleFD, gpsModuleFlags | O_NONBLOCK);
}

/** Relays mavlink packet to low level control system
*
*/
void sendPacketToLowLevel(void) {
    uint8_t uartBuf[BUFFER_LENGTH];
    uint16_t buffer_used;

    memset(uartBuf, 0, sizeof(uartBuf));
    buffer_used = mavlink_msg_to_send_buffer(uartBuf, &msg);

    write(lowLevelControlFD, uartBuf, buffer_used);
}

/** Receives mavlink packets from low level control system
*
* Parses incoming packets from UART interface to low level control system.
* If power system status packet is received, grab the current battery voltage.
*/
void readLowLevelVoltageLevel(void) {
    int incomingSize;
    int i;

    uint8_t uartBuf[BUFFER_LENGTH];
    memset(uartBuf, 0, sizeof(uartBuf));
    incomingSize = readFromDevice(lowLevelControlFD, &uartBuf, BUFFER_LENGTH);

    if (incomingSize) {
        for (i = 0; i < incomingSize; ++i) {
            uint8_t receivedMessage = mavlink_parse_char(MAVLINK_COMM_0,
                                                         uartBuf[i],
                                                         &msg,
                                                         &status);

            if (receivedMessage && (msg.msgid == MAVLINK_MSG_ID_POWER_STATUS)) {
                voltageLevel = mavlink_msg_power_status_get_Vservo(&msg);
            } else {}
        }
    } else {}
}

/** Grab messages from low level control system's uart debug interface and log
*   to stdout and file.
*/
void getUartDebug(void) {
    uint8_t uartBuf[BUFFER_LENGTH];
    memset(uartBuf, 0, sizeof(uartBuf));
    readFromDevice(lowLevelDebugFD, &uartBuf, BUFFER_LENGTH);
    printf("\nArduino message", uartBuf);
    fprintf(logFile, "\nArduino message", uartBuf);
}

/** Processes incoming data from gps uart interface
*
* Incoming data is placed into a buffer and complete strings are sent to
* parser \c processGpsMessage. Excess data in buffer is saved for next time
* through the function.
*/
void readGpsMessages(void) {
    int amountLeft = GPS_BUFFER_SIZE - gpsBufferUsed;
    int ix, lastLineStart;
    
    if (amountLeft > 0) {
        int amountRead = read(gpsModuleFD, gpsBuffer, amountLeft);
        if (amountRead > 0) {
            gpsBufferUsed += amountRead;
        } else if (amountRead < 0) {
            if (errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK) {
                warn("gps: read");
            }
        }
    }

    // Look for lines delimited by CR and/or LF and pass them to processGpsMessage()
    ix = 0;
    lastLineStart = 0;
    while (ix < gpsBufferUsed) {
        if (gpsBuffer[ix] == '\r' || gpsBuffer[ix] == '\n') {
            if (lastLineStart + 1 < ix) {
                processGpsMessage(gpsBuffer + lastLineStart, ix - lastLineStart);
                lastLineStart = ix + 1;
            }
        }
        ix ++;
    }

    if (lastLineStart > 0) {
        // Move any unprocessed partial line to the front of the buffer.
        memmove(gpsBuffer, gpsBuffer + lastLineStart, gpsBufferUsed - lastLineStart);
        gpsBufferUsed -= lastLineStart;
    } else if (gpsBufferUsed == GPS_BUFFER_SIZE) {
        // RX buffer is full, and we saw no line boundaries in it. We're getting garbage
        // of some sort so just discard it all.
        gpsBufferUsed = 0;
    }
}

/** Processes NMEA messages to determine type
*
* Checks char string starting at \c m and verifies checksum. Message is broken
* into fields and dispatched to proper parser for message type.
*
* @param m char string of NMEA message to be processed
* @param msglen length of char string
*/
void processGpsMessage(char *m, int msglen) {
    char *asterisk, *cursor;
#define MAX_NMEA_FIELDS 25
    const char *fields[MAX_NMEA_FIELDS];
    int fieldCount;

    /* Check whether this line looks like a valid NMEA-0183 message */
    if (msglen < 4 || m[0] != '$') {
        /* Not a message, ignore */
        return;
    } else {}

    asterisk = memchr(m, '*', msglen);
    if (asterisk == (m + msglen - 3)) {
        /* Validate the NMEA checksum */
        uint8_t csum = 0;
        uint8_t receivedCsum;
        const char *p;

        for(p = m+1; p != asterisk; p++)
            csum ^= *(uint8_t *)p;
        
        receivedCsum = fromHex2(asterisk[1], asterisk[2]);
        if (receivedCsum != csum) {
            warnx("GPS: Bad NMEA checksum");
            return;
        } else {}
    } else {
        /* Missing checksum. Not technically required to be sent for all sentences, but it is required for the only sentence we listen for ($GPRMC), so let's require it. */
        warnx("GPS: no checksum in NMEA data");
        return;
    }
    
    /* Terminate the sentence at the checksum. */
    *asterisk = 0;

    fprintf(stderr, "GPS sentence [%s]\n", m);

    /* Split the NMEA message into fields */
    fields[0] = m+1; // first field is sentence's data type, eg "GPRMC"
    fieldCount = 1;
    for(cursor = m+1;
        cursor < asterisk && fieldCount < MAX_NMEA_FIELDS;
        cursor ++) {
        if (*cursor == ',') {
            *cursor = 0;
            fields[fieldCount ++] = cursor+1;
        }
    }

    /* Dispatch to a routine based on the sentence's data type */
    if (!strcmp(fields[0], "GPRMC")) {
        processGPRMC(fields, fieldCount);
    } else {
        /* We could process other messages here as well if we wanted to */
    }
}

/** Parse GPRMC NMEA messages to extract readable data
*
* Lat/lon fields are converted into floats and stored in position variable.
*
* Fields of the GPRMC sentence:
*
* \code
*
* $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
*
* Field | Use
* ------|-------------------------------------
* 0     | "GPRMC"
* 1     | HHMMSS  Fix taken at time (UTC)
* 2     | Status A=active or V=Void
* 3,4   | Latitude (decimal degrees, E or W)
* 5,6   | Longitude (deimal degrees, N or S)
* 7     | Speed over the ground in knots
* 8     | Track angle in degrees True
* 9     | DDMMYY  Date of fix
* 10,11 | Magnetic variation (degrees, E or W)
*
* \endcode
*
* @param fields Pointer to char pointer containing GPRMC message
* @param fieldCount number of fields in message
*/
void processGPRMC(const char **fields, int fieldCount) {
    double lat, lon, groundspeed;
    
    if (fieldCount < 9 || !*(fields[3]) || !*(fields[5])) {
        /* Missing or empty fields */
        return;
    }

    lat = atof(fields[3]);
    if (fields[4][0] == 'S')
        lat = -lat;

    lon = atof(fields[5]);
    if (fields[6][0] == 'W')
        lon = -lon;

    groundspeed = atof(fields[7]);

    position.lat = lat / 100;
    position.lon = lon / 100;
    position.vx  = groundspeed;
    fprintf(stderr, "  lat=%f  lon=%f  vx=%d\n", position.lat, position.lon, position.vx);
}

float stringToFloat(position_string_t pos) { // char* s
	long  integer_part = 0;
	float decimal_part = 0.0;
	float decimal_pivot = 0.1;
	uint8_t isdecimal = 0;
    uint8_t isnegative = 0;
	char *c;

    c = pos.c;
	while (*c != ',')  { // ( c = *s++)
		// skip special/sign chars
		if (*c == '-') {
			isnegative = 1;
			c++;
			continue;
		} else {}
		if (*c == '+') {
			c++;
			continue;
		} else {}
		if (*c == '.') {
			isdecimal = 1;
			c++;
			continue;
		} else {} if (!isdecimal) { integer_part = (10 * integer_part) + (*c - 48);
		}
		else {
			decimal_part += decimal_pivot * (float)(*c - 48);
			decimal_pivot /= 10.0;
		}

        c++;
	}
	// add integer part
	decimal_part += (float)integer_part;
	
	// check negative
	if (isnegative)  decimal_part = - decimal_part;

	return decimal_part;
}

/** Reads char from file device
*
* Reads char from device, if no chars available, returns 0
*
* @param f file device to read from
* @param buf buffer to place chars into
* @param buflen max length of buffer
* @return number of chars written to buf
*/
int readFromDevice(int f, char buf[], int buflen) {
    int cread; 
    cread = 0;

    cread = read(f, buf, buflen);

    if (cread >= 0) {
        return cread;
    } else {
        return 0;
    }
}

/** Convert ascii hex char into binary hex value
*
* Converts ascii value 0-9, a/A-f/F to binary equivalent.
* If ascii char is not representative of a hex value, function returns -1.
*
* @param c ASCII hex char
* @return binary value represented by ascii hex char
*/
static short fromHexCh(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return -1;
    }
}

/** Converts two ascii hex chars into binary equivalent
*
* Converts ascii chars 00-ff/FF to binary equivalent by passing two chars
* individually as arguments into function.
* If ascii chars are not representative of a hex value, function returns -1.
*
* @param hh High ascii nibble of hex value
* @param ll Low ascii nibble of hex value
* @return binary value represented by ascii hex chars
*/
int fromHex2(char hh, char ll)
{
    short h = fromHexCh(hh);
    short l = fromHexCh(ll);
    if (h >= 0 && l >= 0) {
        return (h * 16 + l);
    } else {
        return -1;
    }
}

/*****************************************************************************/
/* Timeval utilities                                                         */
/*****************************************************************************/

/** Compares two timevals.
*
* Returns 1 if a>b, -1 if a<b, and 0 if a==b.
*
* @param a first time
* @param b second time
* @return comparison result
*/
int timeval_cmp(struct timeval a, struct timeval b)
{
    if (a.tv_sec > b.tv_sec) {
        return 1;
    } else if (a.tv_sec < b.tv_sec) {
        return -1;
    } else if (a.tv_usec > b.tv_usec) {
        return 1;
    } else if (a.tv_usec < b.tv_usec) {
        return -1;
    } else {
        return 0;
    }
}

/** Sets timeval tv to timeval b if tv exceeds b
* 
* If tv > b then set tv to b
*
* @param tv timeval to be overwritten if it exceeds b
* @param b timeval to check against
*/
void timeval_min(struct timeval *tv, struct timeval b)
{
    if (timeval_cmp(*tv, b) > 0) {
        *tv = b;
    } else {}
}

/** Compare deadline against now.
*
* If deadline is in the past, return 1.
* If it's in the future, adjust *timeout to be no longer than the
* time from now until deadline.
*
* The deadline value has two special cases:
* - {0,-1}   -->   unset. This deadline is never reached.
* - {0,-2}   -->   immediate. This deadline has always passed.
*
* @param now current time
* @param deadline anticipated deadline
* @param timeout time until deadline passes
* @return temporal relation to deadline
*/
int checkDeadline(struct timeval now, struct timeval deadline, struct timeval *timeout) {
    if (deadline.tv_sec == 0 && deadline.tv_usec < 0) {
        if (deadline.tv_usec == -1) {
            /* No deadline is set */
            return 0;
        } else {
            /* "Immediate" deadline */
            return 1;
        }
    } else if (deadline.tv_sec < now.tv_sec) {
        /* Decidedly in the past */
        return 1;
    } else if (deadline.tv_sec == now.tv_sec) {
        /* Possibly in the past --- check the usecs */
        if (deadline.tv_usec <= now.tv_usec) {
            return 1;
        } else {
            struct timeval waittime;
            waittime.tv_sec = 0;
            waittime.tv_usec = deadline.tv_usec - now.tv_usec;
            timeval_min(timeout, waittime);
            return 0;
        }
    } else /* deadline.tv_sec > now.tv_sec */ {
        /* Definitely in the future */
        struct timeval waittime;
        waittime.tv_sec = deadline.tv_sec - now.tv_sec;
        if (deadline.tv_usec < now.tv_usec) {
            /* Borrow the 1 ... sec is guaranteed to be positive because of
             * the conditional above, so this is safe from underflow
             */
            waittime.tv_sec --;
            waittime.tv_usec = 1000000 + deadline.tv_usec - now.tv_usec;
        } else {
            waittime.tv_usec = deadline.tv_usec - now.tv_usec;
        }
        timeval_min(timeout, waittime);
        return 0;
    }
}

/** Adds "increment" seconds to a timeval.
*
* If the timeval is TV_DEADLINE_{IMMEDIATE,UNSET} it is passed through unchanged.
*
* @param tv timeval to increment
* @param increment amount of seconds to increment tv by
* @return new incremented timeval
*/
struct timeval timevalAddf(struct timeval tv, double increment) {
    double isec;

    if (tv.tv_sec == 0 && tv.tv_usec < 0) {
        return tv;
    }

    increment = modf(increment, &isec);
    tv.tv_sec += (int)isec;
    tv.tv_usec += lrint(1e6 * increment);
    if (tv.tv_usec > 1000000) {
        tv.tv_sec ++;
        tv.tv_usec -= 1000000;
    }

    return tv;
}

/** Increment deadline by increment seconds.
*
* Additionally, if the resulting deadline is less than noLessThan seconds
* from tv, extend it even further so that it is.
*
* If the deadline is either of the special cases that checkDeadline()
* handles, pretend that it was equal to tv.
*
* @param deadline anticipated deadline
* @param tv current time
* @param increment number of seconds to increment deadline by
* @param noLessThan minimum accepted time until deadline passes
* @return new incremented deadline
*/
struct timeval nextDeadline(struct timeval deadline, struct timeval tv, double increment, double noLessThan) {
    struct timeval slidingDeadline;

    if (deadline.tv_sec == 0 && deadline.tv_usec < 0) {
        /* An unset deadline. Pretend that it fired just now. */
        deadline = tv;
    }

    deadline = timevalAddf(deadline, increment);
    slidingDeadline = timevalAddf(tv, noLessThan);

    if (timeval_cmp(deadline, slidingDeadline) >= 0) {
        /* The next deadline is sufficiently far in the future. Use it. */
        return deadline;
    } else {
        /* The next deadline is too soon (or possibly in the past). Extend it to now+noLessThan. */
        return slidingDeadline;
    }
}
