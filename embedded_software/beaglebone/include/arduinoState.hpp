/******************************************************************************
 * Hackerboat Beaglebone types module
 * arduinoState.hpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Jan 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef ARDUINOSTATE_H
#define ARDUINOSTATE_H

#include <time.h>
#include <string>
#include "stateStructTypes.hpp" 
#include "boneState.hpp"
#include "location.hpp"

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
		
		bool populate (void);	/**< Populate the object from the named interface */
		bool setCommand (arduinoStateEnum c);
		
		// Command functions...
		bool writeBoneState(boneStateClass::boneStateEnum s);
		bool writeCommand(void);
		int16_t writeThrottle(void);
		double writeHeadingTarget(void);
		double writeHeadingDelta(double delta);
		bool heartbeat(void);
		
		bool 				popStatus;				/**< State of whether the last call to populate() succeeded or failed */
		timespec			uTime;					/**< Time the record was made */
		arduinoStateEnum 	state;					/**< The current state of the boat                    */
		arduinoStateEnum	command;				/**< Last state command received by the Arduino */
		int8_t		 		throttle;   			/**< The current throttle position                    */
		boneStateClass::boneStateEnum 	bone;		/**< The current state of the BeagleBone                */
		orientationClass	orientation;			/**< The current accelerometer tilt and magnetic heading of the boat  */
		float 				headingTarget;			/**< The desired magnetic heading                     */  
		float 				internalVoltage;		/**< The battery voltage measured on the control PCB          */
		float 				batteryVoltage;			/**< The battery voltage measured at the battery            */
		float				motorVoltage;
		bool				enbButton;				/**< State of the enable button. off = 0; on = 0xff           */
		bool				stopButton;				/**< State of the emergency stop button. off = 0; on = 0xff       */
		long 				timeSinceLastPacket;	/**< Number of milliseconds since the last command packet received    */
		long 				timeOfLastPacket;		/**< Time the last packet arrived */
		long 				timeOfLastBoneHB;	
		long 				timeOfLastShoreHB;
		string				stateString;
		string 				boneStateString;
		string				commandString;
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
		bool	 			horn;
		bool				motorDirRly;
		bool				motorWhtRly;
		bool				motorYlwRly;
		bool				motorRedRly;
		bool				motorRedWhtRly;
		bool				motorRedYlwRly;
		bool				servoPower;
		long 				startStopTime;
		long				startStateTime;
		arduinoStateEnum	originState;
		
		static const string const arduinoStates[] = {
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
		static const uint8_t 	arduinoStateCount = 11;
		
	private:
		bool 					setState (arduinoStateEnum s);
		bool 					setBoneState (boneStateClass::boneStateEnum s);
		string					write(string func, string query);		/**< Write to a function on the Arduino */
		
};

#endif /* ARDUINOSTATE_H */
