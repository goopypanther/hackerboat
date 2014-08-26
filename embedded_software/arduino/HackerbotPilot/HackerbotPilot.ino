#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include "mavlink.h"

/** @file */

#define FAULT_LOW_BAT          0x0001			/**< Low battery fault bit 	*/
#define FAULT_SENSOR           0x0002			/**< Sensor fault bit 		*/
#define FAULT_NO_SIGNAL        0x0004			/**< No signal fault bit 	*/
#define FAULT_BB_FAULT         0x0008			/**< Beaglebone fault bit 	*/
#define FAULT_NVM              0x0010			/**< NVM fault bit 			*/

const int32_t sensorTestPeriod =     5000;		/**< Period to check for sensor deviations, in ms 				*/
const int32_t signalTestPeriod =     60000;		/**< Period to wait for Beaglebone signal 						*/
const int32_t startupTestPeriod =    65000;		/**< Period to stay in the self-test state 						*/
const double compassDeviationLimit = 10.0;		/**< Limit of compass swing, in degrees, during test period 	*/
const double tiltDeviationLimit =    15.0;		/**< Limit of tilt in degrees from horizontal during test		*/
const double rateDeviationLimit =    15.0;		/**< Gyro rate limit, in degrees. Currently unused. 			*/
const double testVoltageLimit =      12.0;		/**< Voltage limit during powerup test							*/
const double serviceVoltageLimit =   10.0;		/**< Voltage limit is service									*/

const uint32_t enbButtonTime =       10000;		/**< Time the enable button needs to be pressed, in ms, to arm the boat		*/
const uint32_t stopButtonTime =      1000;		/**< Time the stop button needs to be pressed, in ms, to disarm the boat	*/
const uint32_t disarmedPacketTimeout = 60000;	/**< Connection timeout, in ms, in the disarmed state						*/
const uint32_t armedPacketTimeout =  60000;		/**< Connection timeout, in ms, in the armed state							*/
const uint32_t activePacketTimeout = 300000;	/**< Connection timeout, in ms, in the active state							*/

const uint8_t servoEnable =          2;			/**< Enable pin for the steering servo power supply 	*/
const uint8_t steeringPin =          3;			/**< Steering servo control pin							*/
const uint8_t internalBatVolt =      A0;		/**< Internal battery voltage pin						*/
const double internalBatVoltMult =   1.0;		/**< Internal battery voltage multiplier				*/
const uint8_t relayDir =             52;		/**< Pin to control motor direction. LOW = forward, HIGH = reverse 		*/
const uint8_t relayDirFB =           53;		/**< Motor direction relay wraparound pin			*/
const uint8_t relaySpeedWht =        51;		/**< Motor relay white								*/
const uint8_t relaySpeedWhtFB =      50;		/**< Motor relay white wraparound					*/
const uint8_t relaySpeedYlw =        48;  		/**< Motor relay yellow								*/
const uint8_t relaySpeedYlwFB =      49;		/**< Motor relay yellow wraparound					*/
const uint8_t relaySpeedRed =        47;  		/**< Motor relay red								*/
const uint8_t relaySpeedRedFB =      46;		/**< Motor relay red wraparound						*/
const uint8_t relaySpeedRedWht = 	 44;		/**< Red-White motor crossover relay				*/
const uint8_t relaySpeedRedWhtFB = 	 45;		/**< Red-White motor crossover relay wraparound		*/
const uint8_t relaySpeedRedYlw = 	 43;		/**< Red-Yellow motor crossover relay				*/
const uint8_t relaySpeedRedYlwFB = 	 42;		/**< Red-Yellow motor crossover relay wraparound	*/


const uint16_t sendDelay =           250;		/**< Time in ms between packet transmissions 		*/
//const uint32_t lightTimeout =        1490000;
//const uint32_t ctrlTimeout =         1500000;
//const uint16_t lowVoltCutoff =       750;
//const uint8_t relayAux1 =      47;

/**
 * @brief An enum to store the current state of the boat.
 */
typedef enum boatState {
  BOAT_POWERUP, 		/**< The boat enters this state at the end of initialization */
  BOAT_ARMED,			/**< In this state, the boat is ready to receive go commands over RF */
  BOAT_SELFTEST,		/**< After powerup, the boat enters this state to determine whether it's fit to run */
  BOAT_DISARMED,		/**< This is the default safe state. No external command can start the motor */
  BOAT_ACTIVE,			/**< This is the normal steering state */
  BOAT_LOWBATTERY,		/**< The battery voltage has fallen below that required to operate the motor */
  BOAT_FAULT,			/**< The boat is faulted in some fashion */
  BOAT_SELFRECOVERY		/**< The Beaglebone has failed and/or is not transmitting, so time to self-recover*/
} boatState;				

/**
 * @brief An enum to store the current throttle state.
 */
typedef enum throttleState {
  FWD5,		/**< Full forward speed. Red, white, and yellow all tied to V+, black tied to V- */
  FWD4,		/**< Motor forward 4 */
  FWD3,		/**< Motor forward 3 */
  FWD2,		/**< Motor forward 2 */
  FWD1,		/**< Motor forward 1 */
  STOP,		/**< Motor off */
  REV1,		/**< Motor reverse 1 */
  REV2,		/**< Motor forward 2 */
  REV3		/**< Full reverse speed */
} throttleState;

/**
 * @brief External commands for state transitions, received from the Beaglebone
 */
typedef enum stateCmd {
  CMD_NONE,		/**< No command 											*/
  CMD_DISARM,	/**< Stop moving and return to disarmed state 				*/
  CMD_ACTIVE,	/**< Go from armed to moving 								*/
  CMD_TEST,		/**< Initiate self test (used to clear faults in service) 	*/
  CMD_FAULT		/**< Beaglebone faulted 									*/        
} stateCmd;

/**
 * @brief Structure to hold the boat's state data 
 */
typedef struct boatVector {
  boatState state;				/**< The current state of the boat 										*/
  throttleState throttle;		/**< The current throttle position 										*/
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
void lightControl(boatState state);
int output (throttleState * throttle, double error);
int writeMavlinkPackets (boatVector * thisBoat, double batCurrent, double motVoltage, double motCurrent, long * lastPacketOut);
int writeNVM (boatState state, throttleState throttle, double heading);

void setup (void) {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(servoEnable, OUTPUT);
  pinMode(steeringPin, OUTPUT);
  pinMode(relayDir, OUTPUT);
  pinMode(relayDirFB, INPUT);
  pinMode(relaySpeed1, OUTPUT);
  pinMode(relaySpeed1FB, INPUT);
  pinMode(relaySpeed3, OUTPUT);
  pinMode(relaySpeed3FB, INPUT);
  pinMode(relaySpeed5, OUTPUT);
  pinMode(relaySpeed5FB, INPUT); 
  //pinMode(relayAux1, OUTPUT);
  
  digitalWrite(servoEnable, HIGH);
  digitalWrite(relayDir, LOW);
  digitalWrite(relaySpeed1, LOW);
  digitalWrite(relaySpeed3, LOW);
  digitalWrite(relaySpeed5, LOW);
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
  static double headingCmd = emergencyHeading;
  stateCmd cmd = CMD_NONE;
  double batCurrent;
  double motVoltage;
  double motCurrent;
  static boatState lastState = boat.state;
  
  lastState = boat.state;
  boat.timeSinceLastPacket = getPackets(&boat, &cmd);
  if (getSensors (&boat, &batCurrent, &motVoltage, &motCurrent)) {
    boat.state = BOAT_FAULT;
    Serial.println("Sensor fault!");
    faultString |= FAULT_SENSOR;
  }
  
  switch (boat.state) {
    case BOAT_POWERUP:
      boat.state = BOAT_SELFTEST;
      break;
    case BOAT_SELFTEST:
      boat.state = executeSelfTest(&boat, lastState, cmd);
      break;
    case BOAT_DISARMED:
      boat.state = executeDisarmed(&boat, lastState, cmd);
      break;
    case BOAT_ARMED:
      boat.state = executeArmed(&boat, lastState, cmd);
      break;
    case BOAT_ACTIVE:
      boat.state = executeActive(&boat, lastState, cmd);
      break;
    case BOAT_LOWBATTERY:
      boat.state = executeLowBattery(&boat, lastState, cmd);
      break;
    case BOAT_FAULT:
      boat.state = executeFault(&boat, lastState, cmd);
      break;
    case BOAT_SELFRECOVERY:
      boat.state = executeSelfRecovery(&boat, lastState, cmd);
      break;
    default:
      break;
  }
  
  lightControl(boat.state);
  output(&(boat.throttle), getHeadingError(boat.orientation.heading, headingCmd));
  writeMavlinkPackets(&boat, batCurrent, motVoltage, motCurrent, &lastPacketOut);
  
  
  
  
}
  
  /*
  

  // grab the latest mavlink packet
  int i = 0;
  while (Serial1.available() && (i < 256)) {
    i++;
    if (mavlink_parse_char(0, Serial.read(), &msg, &stat)) {
      if (msg.sysid == 1) {
        Serial.println("Received packet from GCS");
        lastShoreTime = millis();
      } else if (msg.sysid == 2) {
        Serial.println("Received packet from Beaglebone");
        lastCtrlTime = millis();
      } else {
        Serial.print("Received packet from unknown system: ");
        Serial.println(msg.sysid);
      }
      switch (msg.msgid) {
        case MAVLINK_MSG_ID_ATTITUDE_CONTROL:
          Serial.println("Received attitude control packet");
          if (2 == mavlink_msg_attitude_control_get_target(&msg)) {
            targetHeading = mavlink_msg_attitude_control_get_yaw(&msg) + 180;
            throttle = mavlink_msg_attitude_control_get_thrust(&msg);
          } else {
            Serial.print("Target is: ");
            Serial.println(mavlink_msg_attitude_control_get_target(&msg));
          }
          break;
        case MAVLINK_MSG_ID_HEARTBEAT:
          Serial.println("Received heartbeat packet");
          break;
        default:
          Serial.println("Received some other sort of packet");
      }
    } 
  }
  //Serial.println("Serial read");
  
  
  switch (state) {
    
    default:
      state = SELFTEST;
      
  }
  //Serial.println("State machine executed");
  
  steeringPID.Compute();
  steeringServo.write(steeringCmd + 90);
  //Serial.println("Steering complete");
  
  if ((millis() - lastPacketOut) > sendDelay) {
    mavlink_message_t outMsg;
    byte * outBuf;
    uint16_t len;
    static uint8_t blinker = 0;
    
    if ((millis() - lastCtrlTime) > lightTimeout) {
      digitalWrite(relaySpeed5, HIGH); 
    } else {
      if (blinker) {
        digitalWrite(relaySpeed5, LOW); 
        blinker = 0;
      } else {
        digitalWrite(relaySpeed5, HIGH); 
        blinker = 1;
      }
    }
    
    len = mavlink_msg_power_status_pack(2, MAV_COMP_ID_SERVO1, 
      &outMsg, 500, (analogRead(batVolt)*14.7), 0);
    outBuf = (byte *)(&outMsg);
    Serial1.write(outBuf, len);
    Serial.print("Current time: "); Serial.println(millis());
    Serial.print("Bat voltage: "); Serial.print(analogRead(batVolt)*14.7); Serial.println("V");
    Serial.print("Current heading: "); Serial.println(orientation.heading);
    Serial.print("Current target: "); Serial.println(targetHeading);
    Serial.print("Current error: "); Serial.println(currentError);
    Serial.print("Steering command: "); Serial.println(steeringCmd);
    Serial.print("State: "); Serial.println(state);
    len = mavlink_msg_attitude_pack(2, MAV_COMP_ID_SERVO1, 
      &outMsg, millis(), ((targetHeading - 180) * (3.1415/180)), 
      (currentError * (3.1415/180)), (orientation.heading * (3.1415/180)),
      0, 0, 0);
    outBuf = (byte *)(&outMsg);
    Serial1.write(outBuf, len);
    lastPacketOut = millis();
  }
  
}*/

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
  uint8_t getButtons;
  
  // get & process the IMU data
  accel.getEvent(&accel_event);
  mag.getEvent(&mag_event);
  dof.accelGetOrientation(&accel_event, &(thisBoat->orientation));
  dof.magTiltCompensation(SENSOR_AXIS_Z, &mag_event, &accel_event);
  dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &(thisBoat->orientation));
  
  // get analog inputs
  thisBoat->internalVoltage = analogRead(internalBatVolt) * internalBatVoltMult;
  
  // get discrete inputs
  
  
  return failCnt;
}

/**
 * @brief Get all of the input sensor values
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
  
  
  return 0;
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
  getSensors (thisBoat, &batCurrent, &motVoltage, &motCurrent);
  if (thisBoat->internalVoltage < testVoltageLimit) {
    faultCnt++;
    faultString |= FAULT_LOW_BAT;
  }
  if ((millis() - startTime) < sensorTestPeriod) {
    if ((thisBoat->orientation.roll > tiltDeviationLimit) || (thisBoat->orientation.pitch > tiltDeviationLimit)) {
      faultCnt++;
      faultString |= FAULT_SENSOR;
    }
  }
  
  // check for a signal from the beaglebone
  if (getPackets(thisBoat, &myCmd) > signalTestPeriod) {
    faultCnt++;
    faultString |= FAULT_NO_SIGNAL;
  }
  if (CMD_FAULT == myCmd) {
    faultCnt++;
    faultString |= FAULT_BB_FAULT;
  }
  
  // if we've reached the end of the test, grab the NVM and figure out what state we're going to end up in
  if ((millis() - startTime) > startupTestPeriod) {
    if (getNVM(&myState, &myThrottle, &myHeading)) {
      faultString |= FAULT_NVM;
      return BOAT_FAULT;
    } else if (faultCnt) {
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

boatState executeDisarmed(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t lastEnbButton = 0;
  static long startEnbTime = millis();
  
  thisBoat->headingTarget = thisBoat->orientation.heading;
  thisBoat->throttle = STOP;
  if (thisBoat->enbButton && !(lastEnbButton)) {
    startEnbTime = millis();
  } else if (!(thisBoat->enbButton)) {
    startEnbTime = millis();
  }
  lastEnbButton = thisBoat->enbButton;
  
  if ((millis() - startEnbTime) > enbButtonTime) return BOAT_ARMED;
  if (thisBoat->timeSinceLastPacket > disarmedPacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
    return BOAT_FAULT;
  }
  return BOAT_DISARMED;
}

boatState executeArmed(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  static uint8_t lastStopButton = 0;
  static long startStopTime = millis();
  
  thisBoat->throttle = STOP;
  if (thisBoat->stopButton && !(lastStopButton)) {
    startStopTime = millis();
  } else if (!(thisBoat->stopButton)) {
    startStopTime = millis();
  }
  lastStopButton = thisBoat->stopButton;
  
  if ((millis() - startStopTime) > stopButtonTime) return BOAT_DISARMED;
  if (thisBoat->timeSinceLastPacket > disarmedPacketTimeout) {
    faultString |= FAULT_NO_SIGNAL;
    return BOAT_FAULT;
  }
  if (thisBoat->internalVoltage < serviceVoltageLimit) {
    faultString |= FAULT_LOW_BAT;
    return BOAT_LOWBATTERY;
  }
  if (CMD_ACTIVE == cmd) return BOAT_ACTIVE;
  return BOAT_ARMED;
}


boatState executeActive(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  return BOAT_DISARMED;
}

boatState executeLowBattery(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  return BOAT_DISARMED;
}

boatState executeFault(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  return BOAT_DISARMED;
}

boatState executeSelfRecovery(boatVector * thisBoat, boatState lastState, stateCmd cmd) {
  return BOAT_DISARMED;
}

void lightControl(boatState state) {
}

int output (throttleState * throttle, double error) {
  return 0;
}

int writeMavlinkPackets (boatVector * thisBoat, double batCurrent, double motVoltage, double motCurrent, long * lastPacketOut) {
  return 0;
}

int writeNVM (boatState state, throttleState throttle, double heading) {
  return 0;
}


