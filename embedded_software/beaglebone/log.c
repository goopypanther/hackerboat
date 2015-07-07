/**
 * @file log.c
 * @brief Output lines to log
 *
 * @author Jeremy Ruhland jeremy ( a t ) goopypanther.org
 * @author Bryan Godbolt godbolt ( a t ) ualberta.ca
 * @author Wim Lewis wiml ( a t ) hhhh.org
 *
 * @license GPL 3.0 
 * @version 1.0
 * @since Jun 15, 2015
 *
 * Handles data output to logfile and stdout.
 * Call \c logOpen() to open log file and \c logLine() to send data to stdout
 * and logfile.
 */

#include "includes.h"

// Defines
#define SHORE_SYSTEM_ID 0

// Function prototypes
void logOpen(const char *logPath);
void logClose(void);
void logStdOut(const char *data, ...);
void logLine(const char *data, ...);
void logPacket(mavlink_message_t *packet);

// Static variables

static FILE *logFile;
static uint32_t logCurrentlyOpen = FALSE;

static const char packetSourceShoreString[] = "Shore";
static const char packetSourceBeagleboneString[] = "Beaglebone";
static const char packetSourceLowLevelString[] = "Low level";
static const char packetSourceUnknownString[] = "Unknown";

static const char packetTypeHeartbeatString[] = "Heartbeat";
static const char packetTypeGpsLocationString[] = "GPS location";
static const char packetTypeScaledImuString[] = "Scaled IMU";
static const char packetTypeNavControllerOutputString[] = "Nav controller output";

static const char packetTypeBatteryStatusString[] = "Battery status";
static const char packetTypeSystemStatusString[] = "System status";
static const char packetTypeGpsStatusString[] = "GPS status";

static const char packetTypeManualSetpointString[] = "Manual setpoint";
static const char packetTypeSetModeString[] = "Set mode";
static const char packetTypeMissionRequestString[] = "Mission request";
static const char packetTypeMissionItemString[] = "Mission item";
static const char packetTypeMissionClearAllString[] = "Mission clear-all";
static const char packetTypeMissionWritePartialListString[] = "Mission write partial list";
static const char packetTypeMissionRequestListString[] = "Mission request list";
static const char packetTypeMissionRequestPartialListString[] = "Mission request partial list";
static const char packetTypeMissionSetCurrentString[] = "Mission set current";
static const char packetTypeMissionCountString[] = "Mission count";
static const char packetTypeMissionItemReachedString[] = "Mission item reached";
static const char packetTypeMissionCurrentString[] = "Mission current";
static const char packetTypeMissionAckString[] = "Mission ACK";

static const char packetTypeCommandLongString[] = "Command long";
static const char packetTypeCommandAckString[] = "Command ACK";

static const char packetTypeParamRequestString[] = "Param Request";
static const char packetTypeParamValueString[] = "Param value";
static const char packetTypeParamRequestReadString[] = "Param request read";

/**
 * Open log file
 *
 * @param logPath string pointer to log file
 */
void logOpen(const char *logPath) {
    logFile = fopen(logPath, "a"); // Open file, append mode
    logCurrentlyOpen = TRUE;
}

/**
 * Close log file
 */
void logClose(void) {
	if (logCurrentlyOpen == TRUE) {

		(void) fclose(logFile);
		logCurrentlyOpen = FALSE;

	} else {}
}

/**
 * Prints data to standard out.
 *
 * @param data printf-compatible string & variables
 */
void logStdOut(const char *data, ...) {
	va_list dataList;

	va_start(dataList, data);
    vprintf(data, dataList);
    va_end(dataList);
}

/**
 * Prints data to standard out and log file, appending a newline.
 *
 * @param data printf-compatible string & variables
 */
void logLine(const char *data, ...) {
	va_list dataList;

	va_start(dataList, data);

    vprintf(data, dataList);
    vfprintf(logFile, data, dataList);

    printf("\n");
    fprintf(logFile, "\n");

    va_end(dataList);
}

/**
 * Logs mavlink packet
 *
 * @param packet Mavlink packet to log
 */
void logPacket(mavlink_message_t *packet) {
	const char *packetType;
	char unknownPacketTypeBuffer[12];
	const char *packetSource;
	nmea_time_t time;

	currentTimeGet(&time); // Get current time

	// Determine if packet came from shore
	if (packet->sysid == SHORE_SYSTEM_ID) {
		packetSource = packetSourceShoreString;

	// Did packet come from beaglebone
	} else if (packet->compid == MAV_COMP_ID_SYSTEM_CONTROL) {
		packetSource = packetSourceBeagleboneString;

	// Did packet come from arduino
	} else if (packet->compid == MAV_COMP_ID_SERVO1 ||
			   packet->compid == MAV_COMP_ID_IMU) {
		packetSource = packetSourceLowLevelString;

	// Did packet come from somewhere else
	} else {
		packetSource = packetSourceUnknownString;
	}

	// Determine packet type
	switch (packet->msgid) {
	case MAVLINK_MSG_ID_MISSION_REQUEST_PARTIAL_LIST:
		packetType = packetTypeMissionRequestPartialListString;
		break;

	case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
		packetType = packetTypeNavControllerOutputString;
		break;

	case MAVLINK_MSG_ID_HEARTBEAT:
		packetType = packetTypeHeartbeatString;
		break;

	case MAVLINK_MSG_ID_COMMAND_ACK:
		packetType = packetTypeCommandAckString;
		break;

	case MAVLINK_MSG_ID_BATTERY_STATUS:
		packetType = packetTypeBatteryStatusString;
		break;

	case MAVLINK_MSG_ID_COMMAND_LONG:
		packetType = packetTypeCommandLongString;
		break;

	case MAVLINK_MSG_ID_MISSION_REQUEST:
		packetType = packetTypeMissionRequestString;
		break;

	case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
		packetType = packetTypeParamRequestString;
		break;

	case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
		packetType = packetTypeGpsLocationString;
		break;

	case MAVLINK_MSG_ID_SYS_STATUS:
		packetType = packetTypeSystemStatusString;
		break;

	case MAVLINK_MSG_ID_MANUAL_SETPOINT:
		packetType = packetTypeManualSetpointString;
		break;

	case MAVLINK_MSG_ID_PARAM_VALUE:
		packetType = packetTypeParamValueString;
		break;

	case MAVLINK_MSG_ID_MISSION_ITEM:
		packetType = packetTypeMissionItemString;
		break;

	case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
		packetType = packetTypeMissionClearAllString;
		break;

	case MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST:
		packetType = packetTypeMissionWritePartialListString;
		break;

	case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
		packetType = packetTypeMissionRequestListString;
		break;

	case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
		packetType = packetTypeMissionSetCurrentString;
		break;

	case MAVLINK_MSG_ID_GPS_STATUS:
		packetType = packetTypeGpsStatusString;
		break;

	case MAVLINK_MSG_ID_MISSION_COUNT:
		packetType = packetTypeMissionCountString;
		break;

	case MAVLINK_MSG_ID_SET_MODE:
		packetType = packetTypeSetModeString;
		break;

	case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
		packetType = packetTypeMissionItemReachedString;
		break;

	case MAVLINK_MSG_ID_SCALED_IMU:
		packetType = packetTypeScaledImuString;
		break;

	case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
		packetType = packetTypeParamRequestReadString;
		break;

	case MAVLINK_MSG_ID_MISSION_CURRENT:
		packetType = packetTypeMissionCurrentString;
		break;

	case MAVLINK_MSG_ID_MISSION_ACK:
		packetType = packetTypeMissionAckString;
		break;

	default:
		// If unknown packet arrived log type number
		snprintf(unknownPacketTypeBuffer, sizeof(unknownPacketTypeBuffer), "Unknown %d", packet->msgid);
		packetType = unknownPacketTypeBuffer;
	    break;
	}

	logLine("%04u-%02u-%02u %02u:%02u:%02u %s packet from %s", time.year, time.month, time.day, time.hour, time.minute, time.second, packetType, packetSource);
}
