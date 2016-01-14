/******************************************************************************
 * Hackerboat Beaglebone types module
 * stateStructTypes.hpp
 * This modules is compiled into the other modules to give a common interface
 * to the database(s)
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include <jansson.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "config.h"

#define STATE_STRING_LEN		30
#define GPS_SENTENCE_LEN		120
#define	FAULT_STRING_LEN		1024
#define NAV_SOURCE_NAME_LEN		30
#define NAV_VEC_LIST_LEN		30

class hackerboatStateClass {
	public:
	
	virtual bool parse (json_t *input);
	virtual bool get (int select);
	virtual unsigned int write (void);
	virtual json_t *pack (void);
	virtual int count (void);
	virtual bool openFile(const char *name, size_t len);
	virtual bool closeFile(void);
	virtual int getSequenceNum (void);
	
	private:
	
	unsigned int sequenceNum;
	char *formatString;
	sqlite3 *db;
	
}

class gpsFixClass : public hackerboatStateClass {
	public:
	
	gpsFixClass (void);
	gpsFixClass (char *sentence, size_t len);			/**< Create a GPS fix from an incoming sentence */
	
	bool readSentence (char *sentence, size_t len);		/**< Populate class from incoming sentence */
	
	unsigned long 	uTime;				/**< Time of fix arrival */
	double			latitude;			/**< Latitude of last fix */
	double			longitude;			/**< Longitude of last fix */
	double			gpsHeading;			/**< True heading, according to GPS */
	double			gpsSpeed;			/**< Speed over the ground */
	char[GPS_SENTENCE_LEN]		GGA;	/**< GGA sentence from GPS */
	char[GPS_SENTENCE_LEN]		GSA;	/**< GSA sentence from GPS */
	char[GPS_SENTENCE_LEN]		GSV;	/**< GSV sentence from GPS */
	char[GPS_SENTENCE_LEN]		VTG;	/**< VTG sentence from GPS */
	char[GPS_SENTENCE_LEN]		RMC;	/**< RMC sentence from GPS */
}

class locationClass : public hackerboatStateClass {
	public:

	locationClass (void);
	locationClass (double lat, double long);
	
	double lat;
	double long;
}

class navVectorClass : public hackerboatStateClass {
	public:
	
	navVectorClass (void);
	navVectorClass (char *src, size_t srcLen, double bearing, double strength);
	
	char[NAV_SOURCE_NAME_LEN]	source
	double 						bearing;
	double				 		strength;
}

class waypointClass : public hackerboatStateClass {
	public:
	
	waypointClass (void);
	waypointClass (locationClass loc, bool stop);
	
	locationClass	location;
	bool			stop;
}

class boneStateClass : public hackerboatStateClass {
	public:
	
	boneStateClass (void);
	
	unsigned long 				uTime;			/**< Time the record was made, in microseconds past the epoch */
	boneStateEnum				state;			/**< current state of the beaglebone */	
	char[STATE_STRING_LEN]		stateString;	/**< current state of the beaglebone, human readable string */
	boneStateEnum				command;		/**< commanded state of the beaglebone */
	char[STATE_STRING_LEN]		commandString;
	arduinoStateEnum			ardState;		/**< current state of the Arduino */
	char[STATE_STRING_LEN]		ardStateString;
	char[FAULT_STRING_LEN]		faultString;	/**< comma separated list of faults */
	gpsVector					gps;
	int 						waypointNext;
	double						waypointStrength;
	double						waypointAccuracy;
	double						waypointStrengthMax;
	bool						offshore;		/**< When set true, the boat will operate autonomously */
}

class navClass : public hackerboatStateClass {
	public:
	
	navClass (void);
	
	bool appendVector (navVectorClass vec);
	bool calc (void);
	
	locationStruct	current;
	locationStruct	target;
	double			magCorrection;	/**< Correction between sensed magnetic heading and true direction */
	navVector		targetVec;
	navVector		total;
	
	private:
	
	navVector[NAV_VEC_LIST_LEN]	navInfluences;	/**< Array to hold the influences of other navigation sources (i.e. collision avoidance) */
	int				influenceCount;
}

class arduinoStateClass : public hackerboatStateClass {
	public:
	
	arduinoStateClass(void);
	
	bool populate (char *interface size_t len);
	
	unsigned long		uTime;					/**< Time the record was made, in microseconds past the epoch */
	arduinoStateEnum 	state;					/**< The current state of the boat                    */
	arduinoStateEnum	command;
	throttleState 		throttle;   			/**< The current throttle position                    */
	boneStateEnum 		bone;					/**< The current state of the BeagleBone                */
	sensors_vec_t 		orientation;			/**< The current accelerometer tilt and magnetic heading of the boat  */
	float 				headingTarget;			/**< The desired magnetic heading                     */  
	float 				internalVoltage;		/**< The battery voltage measured on the control PCB          */
	float 				batteryVoltage;			/**< The battery voltage measured at the battery            */
	float				motorVoltage;
	int					enbButton;				/**< State of the enable button. off = 0; on = 0xff           */
	int	 				stopButton;				/**< State of the emergency stop button. off = 0; on = 0xff       */
	long 				timeSinceLastPacket;	/**< Number of milliseconds since the last command packet received    */
	long 				timeOfLastPacket;		/**< Time the last packet arrived */
	long 				timeOfLastBoneHB;	
	long 				timeOfLastShoreHB;
	char				stateString[30];
	char 				boneStateString[30];
	char				commandString[30];
	int					faultString;			/**< Fault string -- binary string to indicate source of faults */
	float 				rudder;
	int					rudderRaw;
	int					internalVoltageRaw;
	int					motorVoltageRaw;
	float				motorCurrent;
	int					motorCurrentRaw;
	float				Kp;
	float				Ki;
	float				Kd;
	float	 			magX;
	float 				magY;
	float 				magZ;
	float 				accX;
	float 				accY;
	float 				accZ;
	float 				gyroX;
	float 				gyroY;
	float 				gyroZ;
	uint8_t 			horn;
	uint8_t				motorDirRly;
	uint8_t				motorWhtRly;
	uint8_t				motorYlwRly;
	uint8_t				motorRedRly;
	uint8_t				motorRedWhtRly;
	uint8_t				motorRedYlwRly;
	uint8_t				servoPower;
	long 				startStopTime;
	long				startStateTime;
	arduinoStateEnum	originState;
}