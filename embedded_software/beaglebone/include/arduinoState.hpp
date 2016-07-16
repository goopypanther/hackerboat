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
#include "enumtable.hpp"
#include "location.hpp"
#include <jansson.h>
#include "sqliteStorage.hpp"

#define USE_RESULT __attribute__((warn_unused_result))

/**
 * @class arduinoStateClass
 *
 * @brief Class for storing the current state of the Arduino element
 *
 */

class arduinoStateClass : public hackerboatStateClassStorable {
	public:

		typedef arduinoModeEnum Mode;
		static const enumerationNameTable<arduinoModeEnum> modeNames;

		bool populate (void) USE_RESULT;				/** Populate the object */
		bool corePopulate (void) USE_RESULT;			/**< Populate core data only */
		bool setCommand (arduinoModeEnum c);
		virtual void release(void) {
			if (arduinoStorage) {
				arduinoStorage->closeDatabase();
				arduinoStorage = NULL;
			}
		}
		
		// Command functions...
		bool writeBoatMode(boatModeEnum s);
		bool writeCommand(arduinoModeEnum s);
		bool writeCommand(void);
		int16_t writeThrottle(int16_t t);
		int16_t writeThrottle(void);
		double writeHeadingTarget(void);
		double writeHeadingDelta(double delta);
		bool heartbeat(void);

		bool 				popStatus;			/**< State of whether the last call to populate() succeeded or failed */
		timespec			uTime;				/**< Time the record was made */
		arduinoModeEnum 	mode = Mode::NONE;	/**< The current mode of the arduino                    */
		arduinoModeEnum		command = Mode::NONE;	/**< Last state command received by the Arduino */
		int8_t		 		throttle;   			/**< The current throttle position                    */
		boatModeEnum 		boat = boatModeEnum::NONE;	/**< The current mode of the BeagleBone                */
		orientationClass	orientation;			/**< The current accelerometer tilt and magnetic heading of the boat  */
		float 				headingTarget;			/**< The desired magnetic heading                     */
		float 				internalVoltage;		/**< The battery voltage measured on the control PCB          */
		float 				batteryVoltage;			/**< The battery voltage measured at the battery            */
		float				motorVoltage;
		bool				enbButton;			/**< State of the enable button. off = 0; on = 0xff           */
		bool				stopButton;			/**< State of the emergency stop button. off = 0; on = 0xff       */
		long 				timeSinceLastPacket;		/**< Number of milliseconds since the last command packet received    */
		long 				timeOfLastPacket;		/**< Time the last packet arrived */
		long 				timeOfLastBoneHB;
		long 				timeOfLastShoreHB;
		std::string			faultString;			/**< Fault string -- string to indicate source of faults */
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
		long				startModeTime;
		arduinoModeEnum		originMode = Mode::NONE;


		/* Concrete implementations of stateClassStorable */
		virtual bool parse (json_t *input, bool seq);
		virtual json_t *pack (bool seq = true) const;
		virtual bool isValid (void) const;
		
		/* generic Arduino writer */
		json_t	*writeArduino(std::string func, std::string query);		/**< Write to a function on the Arduino */
		
		~arduinoStateClass (void) {release();}
		
	private:
		bool 	setMode (arduinoModeEnum s);
		bool 	setBoatMode (boatModeEnum s);
		int 	openArduinoSerial (void);
		void 	closeArduinoSerial (void);
		hackerboatStateStorage *arduinoStorage = NULL;;
		
		int ard_fd = -1;
		
		
	protected:
		/* Concrete implementations of stateClassStorable */
		virtual hackerboatStateStorage& storage();
};
inline static const std::string& toString(arduinoModeEnum num) {
	return arduinoStateClass::modeNames.get(num);
}
inline static bool fromString(std::string name, arduinoModeEnum *value) {
	return arduinoStateClass::modeNames.get(name, value);
}
inline static json_t *json(arduinoModeEnum num) {
	return json(arduinoStateClass::modeNames.get(num));
}

#undef USE_RESULT

#endif /* ARDUINOSTATE_H */
