#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_NeoPixel.h>
#include "mavlink.h"

/** @file */

#define FAULT_LOW_BAT          0x0001			/**< Low battery fault bit 	*/
#define FAULT_SENSOR           0x0002			/**< Sensor fault bit 		*/
#define FAULT_NO_SIGNAL        0x0004			/**< No signal fault bit 	*/
#define FAULT_BB_FAULT         0x0008			/**< Beaglebone fault bit 	*/
#define FAULT_NVM              0x0010			/**< NVM fault bit 			*/

// calibration constants
// these constants are used to calibrate the accelerometer and magnetometer
#define X_ACCEL_OFFSET          (-0.275)
#define X_ACCEL_GAIN            (0.092378753)
#define Y_ACCEL_OFFSET          (2.825)
#define Y_ACCEL_GAIN            (0.07160759)
#define Z_ACCEL_OFFSET          (-3.925)
#define Z_ACCEL_GAIN            (0.06655574)
#define X_MAG_OFFSET            (-24.725)
#define X_MAG_GAIN              (0.018003421)
#define Y_MAG_OFFSET            (0)
#define Y_MAG_GAIN              (0.017571604)
#define Z_MAG_OFFSET            (-5.92)
#define Z_MAG_GAIN              (0.017253278)


// test limits
const double compassDeviationLimit = 	10.0;	/**< Limit of compass swing, in degrees, during test period 	*/
const double tiltDeviationLimit =    	15.0;	/**< Limit of tilt in degrees from horizontal during test		*/
const double rateDeviationLimit =    	15.0;	/**< Gyro rate limit, in degrees. Currently unused. 			*/
const double testVoltageLimit =     	12.0;	/**< Battery voltage lower limit during powerup test			*/
const double serviceVoltageLimit =   	10.0;	/**< Battery voltage lower limit in service						*/
const double recoverVoltageLimit = 		11.0;	/**< Battery voltage required to recover from low battery state	*/

// time delays
const int32_t sensorTestPeriod = 	    5000;	/**< Period to check for sensor deviations, in ms 							*/
const int32_t signalTestPeriod =     	60000;	/**< Period to wait for Beaglebone signal 									*/
const int32_t startupTestPeriod =    	65000;	/**< Period to stay in the self-test state 									*/
const int32_t enbButtonTime =       	5000;	/**< Time the enable button needs to be pressed, in ms, to arm the boat		*/
const int32_t stopButtonTime =      	250;	/**< Time the stop button needs to be pressed, in ms, to disarm the boat	*/
const int32_t disarmedPacketTimeout = 60000;	/**< Connection timeout, in ms, in the disarmed state						*/
const int32_t armedPacketTimeout =  	60000;	/**< Connection timeout, in ms, in the armed state							*/
const int32_t activePacketTimeout = 	300000;	/**< Connection timeout, in ms, in the active state							*/
const int32_t hornTimeout = 			    2000;	/**< Time in ms to sound the horn for before entering an unsafe state		*/	
const int16_t sendDelay =           	1000;	/**< Time in ms between packet transmissions 								*/
const int16_t flashDelay = 				    500;	/**< Time in ms between light transitions while flashing					*/

// pin mapping
const uint8_t servoEnable =          	2;		/**< Enable pin for the steering servo power supply 	*/
const uint8_t steeringPin =          	3;		/**< Steering servo control pin							*/
const uint8_t internalBatVolt =      	A0;		/**< Internal battery voltage pin						*/
const uint8_t batteryVolt =				A1;		/**< External battery voltage							*/
const uint8_t batteryCurrent =			A10;	/**< External battery current							*/
const uint8_t motorVolt =				A15;	/**< Motor voltage (measured at speed control input)	*/
const uint8_t motorCurrent =			A13;	/**< Motor current (measured at speed control input)	*/
const uint8_t relayDir =             	52;		/**< Pin to control motor direction. LOW = forward, HIGH = reverse 	*/
const uint8_t relayDirFB =           	53;		/**< Motor direction relay wraparound pin				*/
const uint8_t relaySpeedWht =        	51;		/**< Motor relay white									*/
const uint8_t relaySpeedWhtFB =      	50;		/**< Motor relay white wraparound						*/
const uint8_t relaySpeedYlw =        	48;  	/**< Motor relay yellow									*/
const uint8_t relaySpeedYlwFB =      	49;		/**< Motor relay yellow wraparound						*/
const uint8_t relaySpeedRed =        	47;  	/**< Motor relay red									*/
const uint8_t relaySpeedRedFB =      	46;		/**< Motor relay red wraparound							*/
const uint8_t relaySpeedRedWht = 	 	44;		/**< Red-White motor crossover relay					*/
const uint8_t relaySpeedRedWhtFB = 	 	45;		/**< Red-White motor crossover relay wraparound			*/
const uint8_t relaySpeedRedYlw = 	 	43;		/**< Red-Yellow motor crossover relay					*/
const uint8_t relaySpeedRedYlwFB = 	 	42;		/**< Red-Yellow motor crossover relay wraparound		*/
const uint8_t horn =				 	40;		/**< Alert horn 										*/
const uint8_t hornFB = 					41; 	/**< Alert horn wraparound								*/
const uint8_t arduinoLightsPin = 		39;		/**< Arduino state indicator lights pin					*/
const uint8_t boneLightsPin =			38;		/**< Beaglebone state indicator lights pin				*/
const uint8_t enableButton =			37;		/**< Enable button input								*/	
const uint8_t stopButton = 				36;		/**< Stop button input									*/

// pin-associated constants
const uint8_t boneLightCount =			8;		/**< The number of pixels in the BeagleBone light strip	*/	
const uint8_t ardLightCount =			8;		/**< The number of pixels in the Arduino light strip	*/
const int16_t motorCurrentOffset = 		0;		/**< Motor current offset								*/
const double motorCurrentMult =			1.0;	/**< Motor current gain									*/
const double motorVoltMult = 			1.0;	/**< Motor voltage gain									*/
const int16_t batteryCurrentOffset =	0;		/**< Battery current offset								*/
const double batteryCurrentMult =		1.0;	/**< Battery current gain								*/
const double internalBatVoltMult =   	1.0;	/**< Internal battery voltage multiplier				*/
const double batteryVoltMult = 			1.0;	/**< Battery voltage gain								*/

// color definitions
const uint32_t grn = Adafruit_NeoPixel::Color(0, 0xff, 0);			/**< pixel colors for green		*/
const uint32_t red = Adafruit_NeoPixel::Color(0xff, 0, 0);			/**< pixel colors for red		*/
const uint32_t blu = Adafruit_NeoPixel::Color(0, 0, 0xff);			/**< pixel colors for blue		*/
const uint32_t amb = Adafruit_NeoPixel::Color(0xff, 0xbf, 0);		/**< pixel colors for amber		*/
const uint32_t wht = Adafruit_NeoPixel::Color(0xff, 0xff, 0xff);	/**< pixel colors for white		*/

//const uint32_t lightTimeout =        1490000;
//const uint32_t ctrlTimeout =         1500000;
//const uint16_t lowVoltCutoff =       750;
//const uint8_t relayAux1 =      47;

/**
 * @brief An enum to store the current state of the boat.
 */
typedef enum boatState {
  BOAT_POWERUP 		= 0, 	/**< The boat enters this state at the end of initialization */
  BOAT_ARMED 		= 1,	/**< In this state, the boat is ready to receive go commands over RF */
  BOAT_SELFTEST 	= 2,	/**< After powerup, the boat enters this state to determine whether it's fit to run */
  BOAT_DISARMED 	= 3,	/**< This is the default safe state. No external command can start the motor */
  BOAT_ACTIVE 		= 4,	/**< This is the normal steering state */
  BOAT_LOWBATTERY 	= 5,	/**< The battery voltage has fallen below that required to operate the motor */
  BOAT_FAULT 		= 6,	/**< The boat is faulted in some fashion */
  BOAT_SELFRECOVERY = 7		/**< The Beaglebone has failed and/or is not transmitting, so time to self-recover*/
} boatState;				

/**
 * @brief An enum to store the current throttle state.
 */
typedef enum throttleState {
  FWD5 		= 5,	/**< Full forward speed. Red, white, and yellow all tied to V+, black tied to V- */
  FWD4		= 4,	/**< Motor forward 4 */
  FWD3		= 3,	/**< Motor forward 3 */
  FWD2		= 2,	/**< Motor forward 2 */
  FWD1		= 1,	/**< Motor forward 1 */
  STOP		= 0,	/**< Motor off */
  REV1		= 255,	/**< Motor reverse 1 */
  REV2		= 254,	/**< Motor forward 2 */
  REV3		= 253	/**< Full reverse speed */
} throttleState;

/**
 * @brief External commands for state transitions, received from the Beaglebone
 */
typedef enum stateCmd {
  CMD_NONE,		/**< No command 											*/
  CMD_DISARM,	/**< Stop moving and return to disarmed state 				*/
  CMD_ACTIVE,	/**< Go from armed to active 								*/
  CMD_HALT,		/**< Go from active to armed								*/
  CMD_TEST,		/**< Initiate self test (used to clear faults in service) 	*/
} stateCmd;

/**
 * @brief Beaglebone state
 */
typedef enum boneState {
  BONE_POWERUP		= 0,	/**< Initial starting state 				*/
  BONE_SELFTEST		= 1,	/**< Initial self-test						*/
  BONE_DISARMED		= 2,	/**< Disarmed wait state					*/
  BONE_ARMED		= 3,	/**< Beaglebone armed & ready to navigate	*/  
  BONE_WAYPOINT		= 4,	/**< Beaglebone navigating by waypoints		*/
  BONE_STEERING		= 5,	/**< Beaglebone manual steering				*/
  BONE_NOSIGNAL		= 6,	/**< Beaglbone has lost shore signal		*/	
  BONE_FAULT		= 7		/**< Beaglebone faulted 					*/ 
} boneState;

/**
 * @brief Structure to hold the boat's state data 
 */
typedef struct boatVector {
  boatState state;				/**< The current state of the boat 										*/
  throttleState throttle;		/**< The current throttle position 										*/
  boneState	bone;				/**< The current state of the BeagleBone								*/
  sensors_vec_t orientation;	/**< The current accelerometer tilt and magnetic heading of the boat 	*/
  double headingTarget;			/**< The desired magnetic heading 										*/	
  double internalVoltage;		/**< The battery voltage measured on the control PCB 					*/
  double batteryVoltage;		/**< The battery voltage measured at the battery 						*/
  uint8_t enbButton;			/**< State of the enable button. off = 0; on = 0xff 					*/
  uint8_t stopButton;			/**< State of the emergency stop button. off = 0; on = 0xff 			*/
  long timeSinceLastPacket;		/**< Number of milliseconds since the last command packet received 		*/
} boatVector;

Servo steeringServo;		/**< Servo object corresponding to the steering servo 					*/
double currentError;		/**< Current heading error. This is a global so the PID can be as well 	*/
double targetError = 0;		/**< Desired heading error. This is a global so the PID can be as well 	*/
double steeringCmd;			/**< Steering command to be written out to the servo. 					*/
/**
 * @brief PID loop object for driving the steering servo
 */
PID steeringPID(&currentError, &steeringCmd, &targetError, 1, 0.01, 0, REVERSE); 
Adafruit_9DOF dof = 					Adafruit_9DOF(); 						/**< IMU object 							*/
Adafruit_LSM303_Accel_Unified accel = 	Adafruit_LSM303_Accel_Unified(30301);	/**< Accelerometer object 					*/
Adafruit_LSM303_Mag_Unified   mag   = 	Adafruit_LSM303_Mag_Unified(30302);		/**< Magnetometer object 					*/
boatVector boat;																/**< Boat state vector 						*/
const double emergencyHeading = 90;												/**< Emergency heading for self-recovery	*/
uint16_t faultString = 0;														/**< Fault string -- binary string 			*/

// function declarations
double getHeadingError (double heading, double headingSet);
int getSensors (boatVector * thisBoat, double * batCurrent, double * motVoltage, double * motCurrent);
long int getPackets (boatVector * thisBoat, stateCmd * cmd);
int getNVM (boatState * state, throttleState * throttle, double * heading);
uint8_t getButtons (void);
boatState executeSelfTest(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeDisarmed(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeArmed(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeActive(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeLowBattery(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeFault(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeSelfRecovery(boatVector * thisBoat, boatState lastState, stateCmd cmd);
void lightControl(boatState state, boneState bone);
int output (throttleState * throttle, double error);
int writeMavlinkPackets (boatVector * thisBoat, double batCurrent, double motVoltage, double motCurrent, long * lastPacketOut);
int writeNVM (boatState state, throttleState throttle, double heading);
void calibrateMag (sensors_event_t *magEvent);
void calibrateAccel (sensors_event_t *accelEvent);

void setup (void) {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(servoEnable, OUTPUT);
  pinMode(steeringPin, OUTPUT);
  pinMode(relaySpeedWht, OUTPUT);
  pinMode(relaySpeedWhtFB, INPUT);
  pinMode(relaySpeedYlw, OUTPUT);
  pinMode(relaySpeedYlwFB, INPUT);
  pinMode(relaySpeedRed, OUTPUT);
  pinMode(relaySpeedRedFB, INPUT);
  pinMode(relaySpeedRedWht, OUTPUT);
  pinMode(relaySpeedRedWhtFB, INPUT); 
  pinMode(relaySpeedRedYlw, OUTPUT);
  pinMode(relaySpeedRedYlwFB, INPUT);
  pinMode(horn, OUTPUT);
  pinMode(hornFB, INPUT);
  pinMode(arduinoLightsPin, OUTPUT);
  pinMode(boneLightsPin, OUTPUT);
  pinMode(enableButton, INPUT);
  pinMode(stopButton, INPUT);
  
  digitalWrite(servoEnable, LOW);
  digitalWrite(relayDir, LOW);
  digitalWrite(relaySpeedWht, LOW);
  digitalWrite(relaySpeedYlw, LOW);
  digitalWrite(relaySpeedRed, LOW);
  digitalWrite(relaySpeedRedWht, LOW);
  digitalWrite(relaySpeedRedYlw, LOW);
  digitalWrite(horn, LOW);
  //digitalWrite(relayAux1, LOW);
  
  Serial.println("I live!");
  Wire.begin();
  
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
  }
  
  steeringPID.SetSampleTime(100);
  steeringPID.SetOutputLimits(-90.0, 90.0);
  steeringServo.attach(steeringPin);
  steeringPID.SetMode(AUTOMATIC);
  Serial.println("Everything up!");
  
  boat.throttle = STOP;
  boat.state = BOAT_POWERUP;
  
}

void loop (void) {
  //static long iteration = 0;
  static long lastPacketOut = 0;
  //static double headingCmd = emergencyHeading;
  stateCmd cmd = CMD_NONE;
  double batCurrent;
  double motVoltage;
  double motCurrent;
  static boatState lastState = BOAT_POWERUP;
  boatState thisState = BOAT_POWERUP;
  
  
  boat.timeSinceLastPacket = millis() - getPackets(&boat, &cmd);
  if (getSensors (&boat, &batCurrent, &motVoltage, &motCurrent)) {
    lastState = boat.state;
    boat.state = BOAT_FAULT;
    Serial.println("Sensor fault!");
    faultString |= FAULT_SENSOR;
  }
  
  switch (boat.state) {
    case BOAT_POWERUP:
      Serial.println("*** Powering Up ***");
      lastState = boat.state;
      boat.state = BOAT_SELFTEST;
      break;
    case BOAT_SELFTEST:
      thisState = boat.state;
      boat.state = executeSelfTest(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_DISARMED:
      thisState = boat.state;
      boat.state = executeDisarmed(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_ARMED:
      thisState = boat.state;
      boat.state = executeArmed(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_ACTIVE:
      thisState = boat.state;
      boat.state = executeActive(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_LOWBATTERY:
      thisState = boat.state;
      boat.state = executeLowBattery(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_FAULT:
      thisState = boat.state;
      boat.state = executeFault(&boat, lastState, cmd);
      lastState = thisState;
      break;
    case BOAT_SELFRECOVERY:
      thisState = boat.state;
      boat.state = executeSelfRecovery(&boat, lastState, cmd);
      lastState = thisState;
      break;
    default:
      thisState = boat.state;
	    boat.state = executeFault(&boat, lastState, cmd);
      lastState = thisState;
      break;
  }
  
  lightControl(boat.state, boat.bone);
  Serial.print("Heading Error:\t"); Serial.println(getHeadingError(boat.orientation.heading, boat.headingTarget));
  output(&(boat.throttle), getHeadingError(boat.orientation.heading, boat.headingTarget));
  writeMavlinkPackets(&boat, batCurrent, motVoltage, motCurrent, &lastPacketOut);
 
}
 
/** 
 *  @brief generate heading error from current heading and desired heading
 *
 *  @param heading current magnetic heading
 *  @param headingSet desired magnetic heading
 *
 *  @return heading error, adjusted for the fact that it's circular.
 *
 */
double getHeadingError (double heading, double headingSet) {
  double result = headingSet - heading;
  
  if (result > 180) {result -= 360;}
  if (result < -180) {result += 360;}
  
  return result;
}

/**
 *  @brief Get all of the input sensor values
 *
 *	@param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *	@param *batCurrent A pointer to a double allocated for the current flowing out of the battery
 *	@param *motVoltage A pointer to a double allocated for the voltage at the motor speed control relay box
 *  @param *motCurrent A pointer to a double allocated for the current flowing into the motor speed control relay box
 *
 *  @return An integer indicating success or failure. A zero is success; any other value is failure
 *
 */
int getSensors (boatVector * thisBoat, double * batCurrent, double * motVoltage, double * motCurrent) {
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  uint8_t failCnt = 0;
  
  // get & process the IMU data
  accel.getEvent(&accel_event);
  mag.getEvent(&mag_event);
  calibrateMag(&mag_event);
  calibrateAccel(&accel_event);
  dof.accelGetOrientation(&accel_event, &(thisBoat->orientation));
  dof.magTiltCompensation(SENSOR_AXIS_Z, &mag_event, &accel_event);
  dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &(thisBoat->orientation));
  
  // get analog inputs
  thisBoat->internalVoltage = analogRead(internalBatVolt) * internalBatVoltMult;
  thisBoat->batteryVoltage = analogRead(batteryVolt) * batteryVoltMult;
  *batCurrent = (analogRead(batteryCurrent) + batteryCurrentOffset) * batteryCurrentMult;
  *motVoltage = analogRead(motorVolt) * motorVoltMult;
  *motCurrent = (analogRead(motorCurrent) + motorCurrentOffset) * motorCurrentMult;
  
  // get discrete inputs
  thisBoat->enbButton = digitalRead(enableButton);
  thisBoat->stopButton = digitalRead(stopButton);
  
  Serial.println("Incoming sensor readings");
  Serial.print("Internal voltage:\t"); Serial.println(thisBoat->internalVoltage);
  Serial.print("Motor voltage:\t"); Serial.println(*motVoltage);
  Serial.print("Motor current:\t"); Serial.println(*motCurrent);
  Serial.print("Roll:\t\t\t"); Serial.println(thisBoat->orientation.roll);
  Serial.print("Pitch:\t\t\t"); Serial.println(thisBoat->orientation.pitch);
  Serial.print("Heading:\t\t"); Serial.println(thisBoat->orientation.heading);
  Serial.print("Enable:\t\t"); Serial.println(thisBoat->enbButton);
  Serial.print("Stop:\t\t\t"); Serial.println(thisBoat->stopButton);
  Serial.print("Target Heading:\t"); Serial.println(thisBoat->headingTarget);
  
  return failCnt;
}

/**
 * @brief Parse incoming packet
 *
 *	@param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param *cmd A pointer to an enum of type stateCmd, allocated for any incoming command. Should be initialized to CMD_NONE
 *
 *  @return the value of millis() at the receipt of the last valid packet
 *
 */
long int getPackets (boatVector * thisBoat, stateCmd * cmd) {
  static mavlink_message_t msg;
  static mavlink_status_t stat;
  static long lastCtrlTime = millis();
  uint16_t throttleIn = 0;
  uint16_t steeringIn = 0;
  uint16_t bearingIn = 0;
  uint8_t throttleFlag = 0;
  int16_t i = 0;
  
  while (Serial1.available() && (i < 256)) {
    i++;
    if (mavlink_parse_char(0, Serial1.read(), &msg, &stat)) {
      if (msg.sysid == 255) {
        Serial.println("Received packet from GCS");
        lastCtrlTime = millis();
      } else if (msg.sysid == 1) {
        Serial.println("Received packet from Beaglebone");
        lastCtrlTime = millis();
      } else {
        Serial.print("Received packet from unknown system: ");
        Serial.println(msg.sysid);
      }
      switch (msg.msgid) {
		case MAVLINK_MSG_ID_HEARTBEAT:
          Serial.println("Received heartbeat packet");
		  if (msg.sysid == 1) {
			  uint8_t boneMode = mavlink_msg_heartbeat_get_base_mode(&msg);
			  uint8_t boneStatus = mavlink_msg_heartbeat_get_system_status(&msg);
			  if (MAV_STATE_EMERGENCY == boneStatus) {
				  thisBoat->bone = BONE_FAULT;
			  } else if (MAV_MODE_PREFLIGHT == boneMode) {
				  if (MAV_STATE_BOOT == boneStatus) {
					  thisBoat->bone = BONE_SELFTEST;
				  } else if (MAV_STATE_STANDBY == boneStatus) {
					  thisBoat->bone = BONE_DISARMED;
				  }
			  } else if (MAV_MODE_MANUAL_DISARMED == boneMode) {
				  thisBoat->bone = BONE_DISARMED;
			  } else if (MAV_MODE_AUTO_DISARMED == boneMode) {
				  thisBoat->bone = BONE_DISARMED;
			  } else if (MAV_MODE_MANUAL_ARMED == boneMode) {
				  if (MAV_STATE_STANDBY == boneStatus) {
					  thisBoat->bone = BONE_ARMED;
				  } else if (MAV_STATE_ACTIVE == boneStatus) {
					  thisBoat->bone = BONE_STEERING;
				  } else if (MAV_STATE_CRITICAL == boneStatus) {
					  thisBoat->bone = BONE_NOSIGNAL;
				  }
			  } else if (MAV_MODE_AUTO_ARMED == boneMode) {
				  if (MAV_STATE_STANDBY == boneStatus) {
					  thisBoat->bone = BONE_ARMED;
				  } else if (MAV_STATE_ACTIVE == boneStatus) {
					  thisBoat->bone = BONE_WAYPOINT;
				  } else if (MAV_STATE_CRITICAL == boneStatus) {
					  thisBoat->bone = BONE_NOSIGNAL;
				  }
			  }
		  }
          break;
        case MAVLINK_MSG_ID_MANUAL_CONTROL:
		  if ((msg.sysid == 255) && 
			  (BOAT_ACTIVE == thisBoat->state) && 
			  (BONE_STEERING == thisBoat->bone)) {
			  throttleFlag = -1;
			  uint8_t buttonsIn = mavlink_msg_manual_control_get_buttons(&msg);
			  if (1 == buttonsIn) {
				  throttleIn = 100;
			  } else if (4 == buttonsIn) {
				  throttleIn = -100;
			  } else {
				  throttleIn = 0;
			  }
			  steeringIn = mavlink_msg_manual_control_get_r(&msg);
		  }
		  break;
		case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
		  if ((msg.sysid == 1) && 
			  (BOAT_ACTIVE == thisBoat->state) && 
			  (BONE_WAYPOINT == thisBoat->bone)) {
			  throttleFlag = -1;
			  throttleIn = mavlink_msg_nav_controller_output_get_wp_dist(&msg);
			  bearingIn = mavlink_msg_nav_controller_output_get_target_bearing(&msg);
		  }
		  break;
        default:
          Serial.print("Received some other sort of packet: ");
          Serial.println(msg.msgid);
      }
    } 
  }
  
  if (throttleFlag) {
	if (throttleIn <= 0) {
	  if (throttleIn > -1) {
	    thisBoat->throttle = STOP;
	  } else if (throttleIn > -20) {
	    thisBoat->throttle = REV1;
	  } else if (throttleIn > -40) {
	    thisBoat->throttle = REV2;
	  } else if (throttleIn > -60) {
	    thisBoat->throttle = REV3;
	  }
	} else if (throttleIn < 1) {
	  thisBoat->throttle = STOP;
	} else if (throttleIn < 20) {
	  thisBoat->throttle = FWD1;
	} else if (throttleIn < 40) {
	  thisBoat->throttle = FWD2;
	} else if (throttleIn < 60) {
	  thisBoat->throttle = FWD3;
	} else if (throttleIn < 80) {
	  thisBoat->throttle = FWD4;
	} else if (throttleIn < 100) {
	  thisBoat->throttle = FWD5;
	}
	if (MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT == msg.msgid) {
		thisBoat->headingTarget = (double)bearingIn;
	} else if (MAVLINK_MSG_ID_MANUAL_CONTROL == msg.msgid) {
		if (bearingIn > 10) {
			thisBoat->headingTarget = (thisBoat->headingTarget - 2.5);
		} else if (bearingIn < 10) {
			thisBoat->headingTarget = (thisBoat->headingTarget + 2.5);
		}
		if (thisBoat->headingTarget < 0.0) {
			thisBoat->headingTarget += 360.0;
		} else if (thisBoat->headingTarget > 360.0) {
			thisBoat->headingTarget -= 360.0;
		}
	}
  }
  return lastCtrlTime;
}

/**
 *  @brief Retrieve boat state, throttle command, and heading from NVM
 *
 *  @param state A pointer to an allocated instance of the boatState enum. This stores the last state written to NVM
 *  @param throttle A pointer to an allocated instance of the throttleState enum. This stores the last throttle position written to NVM
 *  @param heading A pointer to a double allocated for the last heading written to NVM
 *
 *  @return 0 if all is working, -1 if the NVM read fails for any reason
 *
 */
int getNVM (boatState * state, throttleState * throttle, double * heading) {
  return 0;
}

/** 
 *  @brief Run internal self tests to confirm that the boat's systems are working correctly.
 *
 *	@param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeSelfTest(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t faultCnt = 0;
  static double headingRef;
  static long startTime = millis();
  static uint8_t headingFaultCnt = 0;
  static uint8_t accelFaultCnt = 0;
  static uint8_t gyroFaultCnt = 0;
  static uint8_t signalFaultCnt = 0;
  boatState myState = BOAT_DISARMED;
  throttleState myThrottle = STOP;
  stateCmd myCmd = CMD_NONE;
  double myHeading = 0.0;
  //double intVoltage;
  //double batVoltage;
  double batCurrent;
  double motVoltage;
  double motCurrent;
  //uint8_t enbButton;
  //uint8_t estopButton;
  
  Serial.println("**** Self-testing... ****");
  
  // if we've just entered this state, reset all the counters
  if (lastState != BOAT_SELFTEST) {
    faultCnt = 0; 
    headingRef = thisBoat->orientation.heading;
    headingFaultCnt = 0;
    accelFaultCnt = 0;
    gyroFaultCnt = 0;
    signalFaultCnt = 0;
    startTime = millis();
  }
  
  // check that the sensors are within bounds
  // getSensors (thisBoat, &batCurrent, &motVoltage, &motCurrent);
  if (thisBoat->internalVoltage < testVoltageLimit) {
    faultCnt++;
    faultString |= FAULT_LOW_BAT;
  }
  if ((millis() - startTime) < sensorTestPeriod) {
    if ((thisBoat->orientation.roll > tiltDeviationLimit) || (thisBoat->orientation.pitch > tiltDeviationLimit)) {
      faultCnt++;
      Serial.print("Sensor outside of roll/pitch limits. Measure values roll: ");
      Serial.print(thisBoat->orientation.roll);
      Serial.print(" pitch: ");
      Serial.println(thisBoat->orientation.pitch);
      faultString |= FAULT_SENSOR;
    }
    if (abs(getHeadingError(thisBoat->orientation.heading, headingRef)) > compassDeviationLimit) {
      Serial.print("Compass outside of deviation limits. Compass heading: ");
      Serial.print(thisBoat->orientation.heading);
      Serial.print(" Reference: ");
      Serial.print(headingRef);
      Serial.print(" Error: ");
      Serial.println(getHeadingError(thisBoat->orientation.heading, headingRef));
      faultCnt++;
      faultString |= FAULT_SENSOR;
    }
  }
  
  // check for a signal from the beaglebone
  if ((millis() - getPackets(thisBoat, &myCmd)) > signalTestPeriod) {
    faultCnt++;
    faultString |= FAULT_NO_SIGNAL;
    Serial.print("Signal timeout. Current time: ");
    Serial.print(millis());
    Serial.print(" Last time: ");
    Serial.println(getPackets(thisBoat, &myCmd));
  } else {
    faultString &= !FAULT_NO_SIGNAL;
    Serial.print("Removing signal timeout. Current time: ");
    Serial.print(millis());
    Serial.print(" Last time: ");
    Serial.print(getPackets(thisBoat, &myCmd));
    Serial.print(" Fault string: ");
    Serial.println(faultString);
  }
  if (BONE_FAULT == thisBoat->bone) {
    faultCnt++;
    faultString |= FAULT_BB_FAULT;
  }
  
  // Keep the steering servo powered through here to enable manual inspection
  digitalWrite(servoEnable, HIGH);
  
  // if we've reached the end of the test, grab the NVM and figure out what state we're going to end up in
  if ((millis() - startTime) > startupTestPeriod) {
    if (getNVM(&myState, &myThrottle, &myHeading)) {
      faultString |= FAULT_NVM;
      Serial.print("Got faults on startup, NVM edition. Fault string: ");
      Serial.println(faultString, HEX);
      return BOAT_FAULT;
    } else if (faultCnt) {
      Serial.print("Got faults on startup. Fault string: ");
      Serial.println(faultString, HEX);
      if ((FAULT_BB_FAULT == faultString) || (FAULT_NO_SIGNAL == faultString) || ((FAULT_NO_SIGNAL|FAULT_BB_FAULT) == faultString)) {
        if ((BOAT_ARMED == myState) || (BOAT_ACTIVE == myState) || (BOAT_SELFRECOVERY == myState)) return BOAT_SELFRECOVERY;
        else return BOAT_FAULT;
      } else return BOAT_FAULT;
    } else {
      if ((myState != BOAT_ARMED) && (myState != BOAT_ACTIVE)) myState = BOAT_DISARMED;
    }
    
    return myState;
  } else return BOAT_SELFTEST;
}

/** 
 *  @brief Wait for the enable button to be pressed.
 *
 *	@param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeDisarmed(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t lastEnbButton = 0;
  static long startEnbTime = millis();
  
  Serial.println("**** Disarmed ****");
  
  thisBoat->headingTarget = thisBoat->orientation.heading;
  thisBoat->throttle = STOP;
  if (thisBoat->enbButton && !(lastEnbButton)) {
    startEnbTime = millis();
  } else if (!(thisBoat->enbButton)) {
    startEnbTime = millis();
  }
  lastEnbButton = thisBoat->enbButton;
  
  // Disable steering servo
  digitalWrite(servoEnable, LOW);
  
  if ((millis() - startEnbTime) > enbButtonTime) return BOAT_ARMED;
  if (thisBoat->timeSinceLastPacket > disarmedPacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
    return BOAT_FAULT;
  }
  return BOAT_DISARMED;
}

/** 
 *  @brief Sound a horn for ten seconds to alert the crew, and then wait for the active command
 *
 *  @param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeArmed(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t lastStopButton = 0;
  static long startStopTime = millis();
  static long startStateTime = millis();
  static boatState originState = BOAT_DISARMED;
  
  Serial.println("**** Armed ****");
  
  thisBoat->throttle = STOP;
  // Keep the steering servo powered through here to enable manual inspection
  digitalWrite(servoEnable, HIGH);
  
  // check if the stop button has been pressed, and if so, for how long
  if (thisBoat->stopButton && !(lastStopButton)) {
    startStopTime = millis();
  } else if (!(thisBoat->stopButton)) {
    startStopTime = millis();
  }
  lastStopButton = thisBoat->stopButton;
  if ((millis() - startStopTime) > stopButtonTime) return BOAT_DISARMED;
  
  // check for packet time out
  if (thisBoat->timeSinceLastPacket > armedPacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
	digitalWrite(horn, LOW);
    return BOAT_FAULT;
  }
  
  // check for low voltage
  if (thisBoat->internalVoltage < serviceVoltageLimit) {
    faultString |= FAULT_LOW_BAT;
	digitalWrite(horn, LOW);
    return BOAT_LOWBATTERY;
  }
  
  // if we've just entered this state, reset all the counters
  if (lastState != BOAT_ARMED) {
    startStateTime = millis();
	originState = lastState;
	digitalWrite(horn, HIGH);
  }
  
  // check if we came from a safe state and the horn timeout is over; if not, sound the horn and reject commands
  if (((millis() - startStateTime) < hornTimeout) && (BOAT_DISARMED == originState)) {
	digitalWrite(horn, HIGH);
	return BOAT_ARMED;
  } else {
    digitalWrite(horn, LOW);
	if (BONE_WAYPOINT == thisBoat->bone) return BOAT_ACTIVE;
	if (BONE_STEERING == thisBoat->bone) return BOAT_ACTIVE;
	if (BONE_DISARMED == thisBoat->bone) return BOAT_DISARMED;
	if (BONE_NOSIGNAL == thisBoat->bone) return BOAT_DISARMED;
	if (BONE_FAULT == thisBoat->bone) return BOAT_DISARMED;
  }
  return BOAT_ARMED;
}

/** 
 *  @brief Steer the boat to the course commanded by the Beaglebone
 *
 *  @param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeActive(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t lastStopButton = 0;
  static long startStopTime = millis();
  
  Serial.println("**** Active ****");
  
  // obviously, the steering servo needs to be active
  digitalWrite(servoEnable, HIGH);
  // no reason to sound the horn
  digitalWrite(horn, LOW);
  
  // check if the stop button has been pressed, and if so, for how long
  if (thisBoat->stopButton && !(lastStopButton)) {
    startStopTime = millis();
  } else if (!(thisBoat->stopButton)) {
    startStopTime = millis();
  }
  lastStopButton = thisBoat->stopButton;
  if ((millis() - startStopTime) > stopButtonTime) return BOAT_DISARMED;
  
  // check for packet time out
  if (thisBoat->timeSinceLastPacket > activePacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
    return BOAT_SELFRECOVERY;
  }
  
  // check for command
  if (BONE_ARMED == thisBoat->bone) return BOAT_ARMED;
  if (BONE_DISARMED == thisBoat->bone) return BOAT_DISARMED;
  
  // check for low voltage
  if (thisBoat->internalVoltage < serviceVoltageLimit) {
    faultString |= FAULT_LOW_BAT;
    return BOAT_LOWBATTERY;
  }
  
  return BOAT_ACTIVE;
}

/** 
 *  @brief Wait for the battery to come back
 *
 *  @param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeLowBattery(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static boatState originState = BOAT_LOWBATTERY;
  static uint8_t lastStopButton = 0;
  static long startStopTime = millis();

  Serial.println("**** Low Battery ****");
  
  // obviously, the steering servo needs to be inactive and the motor off
  digitalWrite(servoEnable, LOW);
  thisBoat->throttle = STOP;
  
  // if we've just entered this state, reset all the counters
  if (lastState != BOAT_LOWBATTERY) {
	originState = lastState;
	startStopTime = millis();
  }
  
  // check if the stop button has been pressed, and if so, for how long
  if (thisBoat->stopButton && !(lastStopButton)) {
    startStopTime = millis();
  } else if (!(thisBoat->stopButton)) {
    startStopTime = millis();
  }
  lastStopButton = thisBoat->stopButton;
  if ((millis() - startStopTime) > stopButtonTime) return BOAT_DISARMED;
    
  // check for packet time out
  if (thisBoat->timeSinceLastPacket > activePacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
    return BOAT_SELFRECOVERY;
  }
  
  // check if the battery has recovered
  if (thisBoat->internalVoltage > recoverVoltageLimit) {
    faultString &= ~FAULT_LOW_BAT;
    return originState;
  }

  return BOAT_LOWBATTERY;
}

/** 
 *  @brief The boat has suffered an internal fault. Wait for self-test command.
 *
 *  @param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeFault(boatVector * thisBoat, boatState lastState, stateCmd cmd) {

  Serial.println("**** Fault ****");
  
  thisBoat->throttle = STOP;
  thisBoat->headingTarget = thisBoat->orientation.heading;
  digitalWrite(servoEnable, LOW);
  Serial.print("Fault string: ");
  Serial.println(faultString, HEX);
  if (0 == faultString) return BOAT_DISARMED;
  if (CMD_TEST == cmd) return BOAT_SELFTEST;
  
  return BOAT_FAULT;
}

/** 
 *  @brief The Beaglebone has stopped transmitting or entered a fault state. Time to steer a safe course to shore.
 *
 *  @param *thisBoat A pointer to the state of the boat, of type structure boatVector. Members of this struct are modified here
 *  @param lastState The state of the control system on the last tick
 *  @param cmd Incoming command from the BeagleBone. No effect in this state.
 *
 *  @return the control system state on the next tick
 *
 */
boatState executeSelfRecovery(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static boatState originState = BOAT_SELFRECOVERY;
  static uint8_t lastStopButton = 0;
  static long startStopTime = millis();

  Serial.println("**** Self Recovery ****");
  
  // obviously, the steering servo needs active and the heading set to the emergency value
  digitalWrite(servoEnable, HIGH);
  thisBoat->throttle = FWD5;
  thisBoat->headingTarget = emergencyHeading;
  
  // if we've just entered this state, reset all the counters
  if (lastState != BOAT_SELFRECOVERY) {
	originState = lastState;
	startStopTime = millis();
  }
  
  // check if the stop button has been pressed, and if so, for how long
  if (thisBoat->stopButton && !(lastStopButton)) {
    startStopTime = millis();
  } else if (!(thisBoat->stopButton)) {
    startStopTime = millis();
  }
  lastStopButton = thisBoat->stopButton;
  if ((millis() - startStopTime) > stopButtonTime) return BOAT_DISARMED;
  
  if (!(faultString & (FAULT_NO_SIGNAL | FAULT_BB_FAULT))) return originState;
  
  return BOAT_SELFRECOVERY;
}

/**
 * @brief Controls the indicator lights on the back of the boat
 * 
 * @param state The state of the Arduino
 * @param bone The state of the BeagleBone
 *
 */
void lightControl(boatState state, boneState bone) {
  static long iteration = 0;
  static long lastChangeTime = millis();
  static uint8_t flashState = 0;
  static Adafruit_NeoPixel ardLights = Adafruit_NeoPixel(ardLightCount, arduinoLightsPin,  NEO_GRB + NEO_KHZ800);
  static Adafruit_NeoPixel boneLights = Adafruit_NeoPixel(boneLightCount, boneLightsPin,  NEO_GRB + NEO_KHZ800);
  uint8_t pixelCounter;
  uint8_t color0;
  //uint8_t color1;
  
  // if this is our first run through, initialize the light strips
  if (0 == iteration) {
  	ardLights.begin();
  	boneLights.begin();
  }
  iteration++;
  
  switch (state) {
    case BOAT_POWERUP:
  	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
    		  flashState = 0;
    		  color0 = 0;
  	    } else {
    		  flashState = 0xff;
    		  color0 = grn;
  	    }
  	  }
  	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
  	    ardLights.setPixelColor(pixelCounter, color0);
  	  }
  	  break;
	  case BOAT_ARMED:
  	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
  	    ardLights.setPixelColor(pixelCounter, blu);
  	  }
  	  break;
	  case BOAT_SELFTEST:
  	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
  	      flashState = 0;
  	      color0 = 0;
        } else {
  	      flashState = 0xff;
  	      color0 = amb;
        }
  	  }
  	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
  	    ardLights.setPixelColor(pixelCounter, color0);
  	  }
  	  break;
	case BOAT_DISARMED:
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
	    ardLights.setPixelColor(pixelCounter, amb);
	  }
	  break;
	case BOAT_ACTIVE:
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
	    ardLights.setPixelColor(pixelCounter, grn);
	  }
	  break;
	case BOAT_LOWBATTERY:
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
		if (pixelCounter % 2) {
		  ardLights.setPixelColor(pixelCounter, grn);
		} else ardLights.setPixelColor(pixelCounter, amb);
	  }
	  break;
	case BOAT_FAULT:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = 0;
	    } else {
		  flashState = 0xff;
		  color0 = red;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
	    ardLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	case BOAT_SELFRECOVERY:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = wht;
	    } else {
		  flashState = 0xff;
		  color0 = red;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
	    ardLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	default:
	  for (pixelCounter = 0; pixelCounter < ardLightCount; pixelCounter++) {
	    ardLights.setPixelColor(pixelCounter, 0);
	  }
	  
	  break;
  }
  
  switch (bone) {
    case BONE_POWERUP:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = 0;
	    } else {
		  flashState = 0xff;
		  color0 = grn;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	case BONE_SELFTEST:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = 0;
	    } else {
		  flashState = 0xff;
		  color0 = amb;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	case BONE_DISARMED:
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, amb);
	  }
	  break;
	case BONE_ARMED:
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, blu);
	  }
	  break;
	case BONE_WAYPOINT:
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, grn);
	  }
	  break;
	case BONE_STEERING:
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
		if (pixelCounter % 2) {
		  boneLights.setPixelColor(pixelCounter, grn);
		} else boneLights.setPixelColor(pixelCounter, blu);
	  }
	  break;
	case BONE_NOSIGNAL:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = wht;
	    } else {
		  flashState = 0xff;
		  color0 = red;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	case BONE_FAULT:
	  if ((millis() - lastChangeTime) > flashDelay) {
        if (flashState) {
		  flashState = 0;
		  color0 = 0;
	    } else {
		  flashState = 0xff;
		  color0 = red;
	    }
	  }
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, color0);
	  }
	  break;
	default:
	  for (pixelCounter = 0; pixelCounter < boneLightCount; pixelCounter++) {
	    boneLights.setPixelColor(pixelCounter, 0);
	  }
	  break;
  }
  
  ardLights.show();
  boneLights.show();
  return;
}

/**
 * @brief Command throttle relays & execute steering PID
 *
 * @param throttle The desired throttle setting
 * @param error The heading error in degrees
 *
 * @return Success or failure; success = 0
 *
 */
int output (throttleState * throttle, double error) {

  Serial.println("Output values: ");
  Serial.print("Throttle:\t"); Serial.println(*throttle);
  Serial.print("Error:\t"); Serial.println(error);
  switch (*throttle) {
    case (FWD5):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, HIGH);
  	  digitalWrite(relaySpeedYlw, HIGH);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (FWD4):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, LOW);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, HIGH);
  	  digitalWrite(relaySpeedRedWht, HIGH);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (FWD3):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, HIGH);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (FWD2):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, LOW);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, HIGH);
  	  break;
    case (FWD1):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, LOW);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (STOP):
  	  digitalWrite(relayDir, LOW);
  	  digitalWrite(relaySpeedWht, LOW);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, LOW);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (REV1):
  	  digitalWrite(relayDir, HIGH);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, LOW);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (REV2):
  	  digitalWrite(relayDir, HIGH);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, LOW);
  	  digitalWrite(relaySpeedYlw, HIGH);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
    case (REV3):
  	  digitalWrite(relayDir, HIGH);
  	  digitalWrite(relaySpeedWht, HIGH);
  	  digitalWrite(relaySpeedRed, HIGH);
  	  digitalWrite(relaySpeedYlw, HIGH);
  	  digitalWrite(relaySpeedRedWht, LOW);
  	  digitalWrite(relaySpeedRedYlw, LOW);
  	  break;
	  default:
	    break;
  }
  
  steeringPID.Compute();
  Serial.print("Steering:\t"); Serial.println(steeringCmd + 90);
  steeringServo.write(steeringCmd + 90);	// The magic number is so that we come out with a correct servo command
  
  return 0;
}

/** 
 * @brief Write out all of the relevant mavlink packets
 * 
 * @param thisBoat State vector for the Arduino
 * @param batCurrent Battery current, in amps
 * @param motVoltage Motor voltage, in volts
 * @param motCurrent Motor current, in amps
 * @param lastPacketOut Time, in ms, that the last packet was transmitted
 *
 */
int writeMavlinkPackets (boatVector * thisBoat, double batCurrent, double motVoltage, double motCurrent, long * lastPacketOut) {
  mavlink_message_t outMsg;
  byte * outBuf;
  uint16_t len;
  uint8_t packetBuffer[sizeof(mavlink_message_t)];
  uint32_t packetBufferFilled;
  
  if ((millis() - *lastPacketOut) > sendDelay) {
    len = mavlink_msg_battery_status_pack(2, MAV_COMP_ID_SERVO1, &outMsg, 0, (uint16_t)(thisBoat->internalVoltage * 1000),  (uint16_t)(thisBoat->batteryVoltage * 1000), 0, 0, 0, 0, 0, 0, 0, 0);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_attitude_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), (thisBoat->headingTarget * (3.1415/180)), 0, (thisBoat->orientation.heading * (3.1415/180)), 0, 0, 0);
    packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	/*len = mavlink_msg_named_value_float_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), 
	                                         "batI", batCurrent);
	packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);*/
	  len = mavlink_msg_named_value_float_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "motorV", motVoltage);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_named_value_float_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "motorI", motCurrent);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
  	len = mavlink_msg_named_value_int_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "enable", thisBoat->enbButton);
  	packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_named_value_int_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "stop", thisBoat->stopButton);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_named_value_int_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "state", thisBoat->state);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_named_value_int_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "throttle", thisBoat->throttle);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_named_value_int_pack(2, MAV_COMP_ID_SERVO1, &outMsg, millis(), "bone", thisBoat->bone);
	  packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  len = mavlink_msg_heartbeat_pack(2, MAV_COMP_ID_SERVO1, &outMsg, MAV_TYPE_SURFACE_BOAT, MAV_AUTOPILOT_GENERIC, 0, 0, MAV_STATE_ACTIVE);
    packetBufferFilled = mavlink_msg_to_send_buffer(packetBuffer, &outMsg);
    Serial1.write(packetBuffer, packetBufferFilled);
	  *lastPacketOut = millis();
  }
  return 0;
}

int writeNVM (boatState state, throttleState throttle, double heading) {
  return 0;
}

void calibrateMag (sensors_event_t *magEvent) {
  float *mag_X, *mag_Y, *mag_Z;
  mag_X = &(magEvent->magnetic.y);
  mag_Y = &(magEvent->magnetic.z);
  mag_Z = &(magEvent->magnetic.x);
  *mag_X += X_MAG_OFFSET;
  *mag_X *= X_MAG_GAIN;
  *mag_Y += Y_MAG_OFFSET;
  *mag_Y *= Y_MAG_GAIN;
  *mag_Z += Z_MAG_OFFSET;
  *mag_Z *= Z_MAG_GAIN;
}

void calibrateAccel (sensors_event_t *accelEvent) {
  float *accel_X, *accel_Y, *accel_Z;
  accel_X = &(accelEvent->acceleration.y);
  accel_Y = &(accelEvent->acceleration.z);
  accel_Z = &(accelEvent->acceleration.x);
  *accel_X += X_ACCEL_OFFSET;
  *accel_X *= X_ACCEL_GAIN;
  *accel_Y += Y_ACCEL_OFFSET;
  *accel_Y *= Y_ACCEL_GAIN;
  *accel_Z += Z_ACCEL_OFFSET;
  *accel_Z *= Z_ACCEL_GAIN;
}
