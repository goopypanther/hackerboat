

#define ARDUINO_BUF_LEN 	4096
#define LOCAL_BUF_LEN		32768
#define MAX_URI_TOKENS		8
#define REST_ID				254
#define REST_NAME			"BoneHackerBoat"
#define MAX_TOKENS			5
#define MAX_TOKEN_LEN		64
#define HASHSEED			0xdeadbeef
#define GNSS_TTY			"/dev/ttyO4"
#define GNSS_BPS			57600
#define ARDUINO_REST_TTY	"/dev/ttyO2"		
#define ARDUINO_LOG_TTY		"/dev/ttyO1"
#define ARDUINO_BPS			115200	
	/**< State machine execution frame length, in nanoseconds */
#define FRAME_LEN_NS		(100000000)		

// File names
#define	GPS_DB_FILE
#define	NAV_DB_FILE
#define	BONE_LOG_DB_FILE
#define ARD_LOG_DB_FILE
#define WP_DB_FILE	

/**
 * @brief Beaglebone state
 */
typedef enum boneStateEnum {
	BONE_START			= 0,  		/**< Initial starting state         */
	BONE_SELFTEST		= 1,  		/**< Initial self-test            */
	BONE_DISARMED		= 2,  		/**< Disarmed wait state          */  
	BONE_FAULT			= 3,		/**< Beaglebone faulted           */ 
	BONE_ARMED			= 4,		/**< Beaglebone armed & ready to navigate */ 
	BONE_MANUAL			= 5,		/**< Beaglebone manual steering       */ 
	BONE_WAYPOINT		= 6,		/**< Beaglebone navigating by waypoints   */
	BONE_NOSIGNAL		= 7,		/**< Beaglebone has lost shore signal    */
	BONE_RETURN			= 8,		/**< Beaglebone is attempting to return to start point */
	BONE_ARMEDTEST		= 9,		/**< Beaglebone accepts all commands that would be valid in any unsafe state */
	BONE_NONE			= 10		/**< State of the Beaglebone is currently unknown	*/
} boneState;

const uint8_t boneStateCount = 11;
const char boneStates[][30] = {
	"Start", 
	"SelfTest", 
	"Disarmed", 
	"Fault",
	"Armed", 
	"Manual", 
	"WaypointNavigation",
	"LossOfSignal", 
	"ReturnToLaunch", 
	"ArmedTest",
	"None"
};

/**
 * @brief An enum to store the current state of the Arduino.
 */
typedef enum arduinoStateEnum {
	BOAT_POWERUP     	= 0,  		/**< The boat enters this state at the end of initialization */
	BOAT_ARMED			= 1,  		/**< In this state, the boat is ready to receive go commands over RF */
	BOAT_SELFTEST   	= 2,  		/**< After powerup, the boat enters this state to determine whether it's fit to run */
	BOAT_DISARMED   	= 3,  		/**< This is the default safe state. No external command can start the motor */
	BOAT_ACTIVE     	= 4,  		/**< This is the normal steering state */
	BOAT_LOWBATTERY   	= 5,  		/**< The battery voltage has fallen below that required to operate the motor */
	BOAT_FAULT    		= 6,  		/**< The boat is faulted in some fashion */
	BOAT_SELFRECOVERY 	= 7,   		/**< The Beaglebone has failed and/or is not transmitting, so time to self-recover*/
	BOAT_ARMEDTEST		= 8,		/**< The Arduino is accepting specific pin read/write requests for hardware testing. */
	BOAT_ACTIVERUDDER	= 9,		/**< The Arduino is accepting direct rudder commands */
	BOAT_NONE			= 10		/**< Provides a null value for no command yet received */
} arduinoState;        

const uint8_t arduinoStateCount = 11;
const char arduinoStates[][30] = {
	"PowerUp", 
	"Armed", 
	"SelfTest", 
	"Disarmed", 
	"Active", 
	"LowBattery", 
	"Fault", 
	"SelfRecovery", 
	"ArmedTest", 
	"ActiveRudder", 
	"None"
};


	