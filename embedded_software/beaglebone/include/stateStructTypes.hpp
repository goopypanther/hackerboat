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

#ifndef STATESTRUCTTYPES_H
#define STATESTRUCTTYPES_H
 
#include <jansson.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <inttypes.h>
#include "config.h"
#include "location.hpp"

// buffer & string sizes
#define STATE_STRING_LEN		30
#define GPS_SENTENCE_LEN		120
#define	FAULT_STRING_LEN		1024
#define NAV_SOURCE_NAME_LEN		30
#define NAV_VEC_LIST_LEN		30

// value limits 

/**
 * @class hackerboatStateClass 
 * 
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 */

class hackerboatStateClass {
	public:
		virtual hackerboatStateClass(void);							
		virtual bool parse (json_t *input);			/**< Populate the object from the given json object */
		virtual json_t *pack (void);				/**< Pack the contents of the object into a json object and return a pointer to that object*/
		virtual bool isValid (void) {return true;};	/**< Tests whether the current object is in a valid state */
		
	protected:	
		virtual const char *getFormatString(void);		/**< Get format string for the object */
};

/**
 * @class hackerboatStateClassStorable 
 * 
 * @brief Base class for holding various types of object used by the core functions of the Hackerboat
 *
 * This base class connects to a database of records containing instances of the object type.  
 *
 */

class hackerboatStateClassStorable : public hackerboatStateClass {
	public:
		virtual hackerboatStateClassStorable(const char *file, size_t len);		/**< Create a state object attached to the given file */
		int32_t getSequenceNum (void) {return _sequenceNum;};					/**< Get the sequenceNum of this object (-1 until populated from a file) */
		virtual bool openFile(const char *name, size_t len);					/**< Open the given database file & store the name */
		virtual bool openFile(void);											/**< Open the stored database file */
		virtual bool closeFile(void);											/**< Close the open file */
		virtual int32_t count (void);											/**< Return the number of records of the object's type in the open database file */
		virtual bool writeRecord (void);										/**< Write the current record to the target database file */
		virtual bool getRecord(int32_t select);									/**< Populate the object from the open database file */
		virtual bool getLastRecord(void);										/**< Get the latest record */
		virtual bool insert(int32_t num) {return false;};						/**< Insert the contents of the object into the database table at the given point */
		virtual bool append(void);												/**< Append the contents of the object to the end of the database table */
		
	protected:
		int32_t 	_sequenceNum = -1;	/**< sequence number */
		char 		*_fileName;			/**< database filename (with path) */
		sqlite3 	*_db;				/**< database handle */
};

/**
 * @class gpsFixClass 
 * 
 * @brief A GPS fix of some type
 *
 * The actual text of each incoming sentence is stored for logging purposes as well. 
 *
 */
 
class gpsFixClass : public hackerboatStateClassStorable {
	public:
		gpsFixClass (void);
		gpsFixClass (char *sentence, size_t len);			/**< Create a GPS fix from an incoming sentence string */
		
		bool readSentence (char *sentence, size_t len);		/**< Populate class from incoming sentence string */
		bool isValid (void);								/**< Check for validity */
		
		unsigned long long			uTime;					/**< Time of fix arrival in microseconds since the epoch */
		double						latitude;				/**< Latitude of last fix */
		double						longitude;				/**< Longitude of last fix */
		double						gpsHeading;				/**< True heading, according to GPS */
		double						gpsSpeed;				/**< Speed over the ground */
		char[GPS_SENTENCE_LEN]		GGA;					/**< GGA sentence from GPS */
		char[GPS_SENTENCE_LEN]		GSA;					/**< GSA sentence from GPS */
		char[GPS_SENTENCE_LEN]		GSV;					/**< GSV sentence from GPS */
		char[GPS_SENTENCE_LEN]		VTG;					/**< VTG sentence from GPS */
		char[GPS_SENTENCE_LEN]		RMC;					/**< RMC sentence from GPS */
	protected:
		const char *getFormatString(void) {return _format;};		/**< Get format string for the object */
	private:
		static const char *_format = "";
};

/**
 * @class waypointClass
 *
 * @brief This is the class for storing & manipulating waypoints
 *
 */
 
class waypointClass : public hackerboatStateClassStorable {
	public:	
		enum actionEnum {
			STOP = 0,						
			HOME = 1,
			CONTINUE = 2,
		};
		
		waypointClass (void);
		waypointClass (locationClass loc);						/**< Create a waypoint at loc */
		waypointClass (locationClass loc, actionEnum action); 	/**< Create a waypoint at loc with action */
		
		waypointClass 	*getNextWaypoint(void);					/**< return the next waypoint to travel towards */
		bool			setNextWaypoint(waypointClass* next);	/**< Set the next waypoint to the given object (works only if it has a sequenceNum > 0; renumber indices as necessary */
		bool			setNextWaypoint(int16_t index);			/**< As above, but set by current index; renumbering proceeds as above */
		int16_t			getNextIndex(void);						/**< Return the index of the next waypoint */
		bool			setAction(actionEnum action);				/**< Set the action to take when this waypoint is reached */
		actionEnum		getAction(void);						/**< Return the action that this waypoint is set to */
		virtual bool insert(int32_t num);						/**< Insert this waypoint into the waypoint list after the given index; renumbers following waypoints */
		virtual bool append(void);								/**< Append this waypoint after the last waypoint in the list. This waypoint takes on the action of the previous last action and sets the previous one to 'continue' */
		
		locationClass	location;				/**< Location of the waypoint */
	protected:
		const char *getFormatString(void) {return _format;};		/**< Get format string for the object */
	private:
		static const char *_format = "";
		int16_t			index = -1;				/**< Place of this waypoint in the waypoint list */ 
		int32_t			nextWaypoint = -1;		/**< _sequenceNum of the next waypoint */
		actionEnum		act = CONTINUE;			/**< Action to perform when reaching a location */	
};

/**
 * @class boneStateClass
 *
 * @brief Class for storing the current state of the Beaglebone element
 *
 */

class boneStateClass : public hackerboatStateClassStorable {
	public:
		/**
		 * @brief Beaglebone state
		 */
		enum boneStateEnum {
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
		};
	
		boneStateClass (void);
		bool insertFault (char* fault, size_t len);	/**< Add the named fault to the fault string. Returns false if fault string is full */
		bool removeFault (char* fault, size_t len);	/**< Remove the named fault to the fault string. Returns false if not present */
		
		unsigned long 				uTime;			/**< Time the record was made, in microseconds past the epoch */
		boneStateEnum				state = BONE_NONE;			/**< current state of the beaglebone */	
		char[STATE_STRING_LEN]		stateString;	/**< current state of the beaglebone, human readable string */
		boneStateEnum				command = BONE_NONE;		/**< commanded state of the beaglebone */
		char[STATE_STRING_LEN]		commandString;	/**< commanded state of the beaglebone, human readable string */
		arduinoStateClass::arduinoStateEnum			ardState;		/**< current state of the Arduino */
		char[STATE_STRING_LEN]		ardStateString;	/**< current state of the Arduino, human readable string */
		char[FAULT_STRING_LEN]		faultString;	/**< comma separated list of faults */
		gpsFixClass					gps;			/**< current GPS position */
		int32_t						waypointNext;	/**< ID of the current target waypoint */
		double						waypointStrength;		/**< Strength of the waypoint */
		double						waypointAccuracy;		/**< How close the boat gets to each waypoint before going to the next one */
		double						waypointStrengthMax;	/**< Maximum waypoint strength */
		bool						autonomous;		/**< When set true, the boat will operate autonomously */	
		static const uint8_t boneStateCount = 11;
		static uint32_t stateHashes[boneStateCount];		/**< All the state names, hashed for easy lookup */
		static const char boneStates[][STATE_STRING_LEN] = {
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
	protected:
		const char *getFormatString(void) {return _format;};		/**< Get format string for the object */
	private:
		void initHashes (void);								/**< Initialize state name hashes */
		static const char *_format = "";
		
};


/**
 * @class arduinoStateClass
 *
 * @brief Class for storing the current state of the Arduino element
 *
 */

class arduinoStateClass : public hackerboatStateClassStorable {
	public:
	
		/**
		 * @brief An enum to store the current state of the Arduino.
		 */
		enum arduinoStateEnum {
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
		};        

		arduinoStateClass(void);
		
		bool populate (char *interface, size_t len);	/**< Populate the object from the named interface */
		
		unsigned long		uTime;					/**< Time the record was made, in microseconds past the epoch */
		arduinoStateEnum 	state;					/**< The current state of the boat                    */
		arduinoStateEnum	command;				/**< Last state command received by the Arduino */
		int8_t		 		throttle;   			/**< The current throttle position                    */
		boneStateClass::boneStateEnum 	bone;		/**< The current state of the BeagleBone                */
		sensors_vec_t 		orientation;			/**< The current accelerometer tilt and magnetic heading of the boat  */
		float 				headingTarget;			/**< The desired magnetic heading                     */  
		float 				internalVoltage;		/**< The battery voltage measured on the control PCB          */
		float 				batteryVoltage;			/**< The battery voltage measured at the battery            */
		float				motorVoltage;
		uint8_t				enbButton;				/**< State of the enable button. off = 0; on = 0xff           */
		uint8_t				stopButton;				/**< State of the emergency stop button. off = 0; on = 0xff       */
		long 				timeSinceLastPacket;	/**< Number of milliseconds since the last command packet received    */
		long 				timeOfLastPacket;		/**< Time the last packet arrived */
		long 				timeOfLastBoneHB;	
		long 				timeOfLastShoreHB;
		char				stateString[STATE_STRING_LEN];
		char 				boneStateString[STATE_STRING_LEN];
		char				commandString[STATE_STRING_LEN];
		uint16_t			faultString;			/**< Fault string -- binary string to indicate source of faults */
		float 				rudder;
		int16_t				rudderRaw;
		int16_t				internalVoltageRaw;
		int16_t				motorVoltageRaw;
		float				motorCurrent;
		int16_t				motorCurrentRaw;
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
	
	protected:
		const char *getFormatString(void) {return _format;};		/**< Get format string for the object */
		
	private:
		static const char *_format = "";	
		static const uint8_t arduinoStateCount = 11;
		static const char arduinoStates[][STATE_STRING_LEN] = {
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
};

#endif /* STATESTRUCTTYPES_H */