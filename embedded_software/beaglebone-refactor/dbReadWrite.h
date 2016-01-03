/******************************************************************************
 * Hackerboat Beaglebone database read/write module
 * dbReadWrite.h
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Dec 2015
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include "config.h"

#define STATE_STRING_LEN		30
#define GPS_SENTENCE_LEN		120
#define	FAULT_STRING_LEN		1024
#define NAV_SOURCE_NAME_LEN		30
#define NAV_VEC_LIST_LEN		30

/**
 * @brief Structure to hold navigation data
 */

typedef struct gpsVector {
	long			sequenceNum;
	unsigned long 	uTime;			/**< Time the record was made, in microseconds past the epoch */
	double			latitude;
	double			longitude;
	double			gpsHeading;		/**< true heading, via GPS */ 
	double			gpsVelocity;
	char[GPS_SENTENCE_LEN]		GGA;			/**< GGA sentence from GPS */
	char[GPS_SENTENCE_LEN]		GSA;
	char[GPS_SENTENCE_LEN]		GSV;
	char[GPS_SENTENCE_LEN]		VTG;
	char[GPS_SENTENCE_LEN]		RMC;
} gpsVector;

bool 	getGPSvector (gpsVector *vec, long seriesSelect, int count);
bool 	writeGPSvector (gpsVector *vec);
json_t 	*packGPSvector (gpsVector vec);
int 	countGPSvector (void);

/** 
 * @brief Structure to hold a position
 */

typedef struct locationStruct {
	double			latitude;
	double			longitude;
} locationStruct;

json_t *packLocationStruct (locationStruct vec);

/** 
 * @brief Structure to hold a nav Vector
 */

typedef struct navVector {
	char[NAV_SOURCE_NAME_LEN]	source
	double 						bearing;
	double				 		strength;
} navVector

json_t *packNavVector (locationStruct vec);

/** 
 * @brief Structure to hold a waypoint
 */

typedef struct waypointStruct {
	int				waypointNumber;		/**< ordinal number of this waypoint */
	locationStruct	location;			/**< location of the waypoint */
	bool			stop;				/**< if this is true and there are no more waypoints, stop here. Otherwise, proceed to the first waypoint */
} waypointStruct;

bool 	getWaypointStruct (waypointStruct *vec, int waypointNumber, int count);
bool 	writeWaypointStruct (waypointStruct *vec);
json_t 	*packWaypointStruct (waypointStruct vec);
int 	countWaypoints (void);
  
 /**
 * @brief Structure to hold the Beaglebone's state data 
 */
 
typedef struct boneVector {
	long						sequenceNum;
	unsigned long 				uTime;			/**< Time the record was made, in microseconds past the epoch */
	boneState					state;			/**< current state of the beaglebone */	
	char[STATE_STRING_LEN]		stateString;	/**< current state of the beaglebone, human readable string */
	boneState					command;		/**< commanded state of the beaglebone */
	char[STATE_STRING_LEN]		commandString;
	arduinoState				ardState;		/**< current state of the Arduino */
	char[STATE_STRING_LEN]		ardStateString;
	char[FAULT_STRING_LEN]		faultString;	/**< comma separated list of faults */
	gpsVector					gps;
	int 						waypointNext;
	double						waypointStrength;
	double						waypointAccuracy;
	double						waypointStrengthMax;
	bool						offshore;		/**< When set true, the boat will operate autonomously */
} boneVector;
 
bool 	getBoneVector 	(boneVector *vec, long seriesSelect, int count); 
bool 	writeBoneVector (boneVector *vec); 
json_t 	*packBoneVector (boneVector vec); 
int 	countBoneVector (void);

/** 
 * @brief Structure to hold navigation data
 */ 
 
typedef struct navStruct {
	long			sequenceNum;
	locationStruct	current;
	locationStruct	target;
	double			magCorrection;	/**< Correction between sensed magnetic heading and true direction */
	navVector		targetVec;
	navVector[NAV_VEC_LIST_LEN]	navInfluences;	/**< Array to hold the influences of other navigation sources (i.e. collision avoidance) */
	navVector		total;
} navStruct;
 
bool 	getNavStruct (navStruct *vec, long seriesSelect, int count); 
bool 	writeNavStruct (navStruct *vec); 
json_t 	*packNavStruct (navStruct vec);
int  	countNavStruct (void);

 /**
 * @brief Structure to hold the Arduino's state data 
 */
 
typedef struct arduinoVector {
	long			sequenceNum;
	unsigned long	uTime;					/**< Time the record was made, in microseconds past the epoch */
	arduinoState 	state;					/**< The current state of the boat                    */
	arduinoState	command;
	throttleState 	throttle;   			/**< The current throttle position                    */
	boneState 		bone;					/**< The current state of the BeagleBone                */
	sensors_vec_t 	orientation;			/**< The current accelerometer tilt and magnetic heading of the boat  */
	float 			headingTarget;			/**< The desired magnetic heading                     */  
	float 			internalVoltage;		/**< The battery voltage measured on the control PCB          */
	float 			batteryVoltage;			/**< The battery voltage measured at the battery            */
	float			motorVoltage;
	int				enbButton;				/**< State of the enable button. off = 0; on = 0xff           */
	int	 			stopButton;				/**< State of the emergency stop button. off = 0; on = 0xff       */
	long 			timeSinceLastPacket;	/**< Number of milliseconds since the last command packet received    */
	long 			timeOfLastPacket;		/**< Time the last packet arrived */
	long 			timeOfLastBoneHB;	
	long 			timeOfLastShoreHB;
	char			stateString[30];
	char 			boneStateString[30];
	char			commandString[30];
	int				faultString;			/**< Fault string -- binary string to indicate source of faults */
	float 			rudder;
	int				rudderRaw;
	int				internalVoltageRaw;
	int				motorVoltageRaw;
	float			motorCurrent;
	int				motorCurrentRaw;
	float			Kp;
	float			Ki;
	float			Kd;
	float 			magX;
	float 			magY;
	float 			magZ;
	float 			accX;
	float 			accY;
	float 			accZ;
	float 			gyroX;
	float 			gyroY;
	float 			gyroZ;
	uint8_t 		horn;
	uint8_t			motorDirRly;
	uint8_t			motorWhtRly;
	uint8_t			motorYlwRly;
	uint8_t			motorRedRly;
	uint8_t			motorRedWhtRly;
	uint8_t			motorRedYlwRly;
	uint8_t			servoPower;
	long 			startStopTime;
	long			startStateTime;
	arduinoState	originState;
} arduinoVector;

bool 	getArduinoVector (arduinoVector *vec, long timeSelect, int count);
bool 	writeArduinoVector (arduinoVector *vec);
json_t 	*packArduinoVector (arduinoVector vec);
int		countArduinoVector (void);