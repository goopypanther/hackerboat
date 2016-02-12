#include <aREST.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_NeoPixel.h>
#include "hardwareConfig.h"
#include "sensorCalibration.h"

#define FAULT_LOW_BAT          0x0001      /**< Low battery fault bit  */
#define FAULT_SENSOR           0x0002     /**< Sensor fault bit     */
#define FAULT_NO_SIGNAL        0x0004     /**< No signal fault bit  */
#define FAULT_BB_FAULT         0x0008     /**< Beaglebone fault bit   */
#define FAULT_NVM              0x0010     /**< NVM fault bit      */
/**
 * @brief An enum to store the current state of the boat.
 */
typedef enum boatState {
  BOAT_POWERUP     = 0,  /**< The boat enters this state at the end of initialization */
  BOAT_ARMED    = 1,  /**< In this state, the boat is ready to receive go commands over RF */
  BOAT_SELFTEST   = 2,  /**< After powerup, the boat enters this state to determine whether it's fit to run */
  BOAT_DISARMED   = 3,  /**< This is the default safe state. No external command can start the motor */
  BOAT_ACTIVE     = 4,  /**< This is the normal steering state */
  BOAT_LOWBATTERY   = 5,  /**< The battery voltage has fallen below that required to operate the motor */
  BOAT_FAULT    = 6,  /**< The boat is faulted in some fashion */
  BOAT_SELFRECOVERY = 7   /**< The Beaglebone has failed and/or is not transmitting, so time to self-recover*/
} boatState;        

/**
 * @brief An enum to store the current throttle state.
 */
typedef enum throttleState {
  FWD5    = 5,  /**< Full forward speed. Red, white, and yellow all tied to V+, black tied to V- */
  FWD4    = 4,  /**< Motor forward 4 */
  FWD3    = 3,  /**< Motor forward 3 */
  FWD2    = 2,  /**< Motor forward 2 */
  FWD1    = 1,  /**< Motor forward 1 */
  STOP    = 0,  /**< Motor off */
  REV1    = 255,  /**< Motor reverse 1 */
  REV2    = 254,  /**< Motor forward 2 */
  REV3    = 253 /**< Full reverse speed */
} throttleState;

/**
 * @brief External commands for state transitions, received from the Beaglebone
 */
typedef enum stateCmd {
  CMD_NONE,   /**< No command                       */
  CMD_DISARM, /**< Stop moving and return to disarmed state         */
  CMD_ACTIVE, /**< Go from armed to active                */
  CMD_HALT,   /**< Go from active to armed                */
  CMD_TEST,   /**< Initiate self test (used to clear faults in service)   */
} stateCmd;

/**
 * @brief Beaglebone state
 */
typedef enum boneState {
  BONE_POWERUP    = 0,  /**< Initial starting state         */
  BONE_SELFTEST   = 1,  /**< Initial self-test            */
  BONE_DISARMED   = 2,  /**< Disarmed wait state          */
  BONE_ARMED    = 3,  /**< Beaglebone armed & ready to navigate */  
  BONE_WAYPOINT   = 4,  /**< Beaglebone navigating by waypoints   */
  BONE_STEERING   = 5,  /**< Beaglebone manual steering       */
  BONE_NOSIGNAL   = 6,  /**< Beaglbone has lost shore signal    */  
  BONE_FAULT    = 7   /**< Beaglebone faulted           */ 
} boneState;

/**
 * @brief Structure to hold the boat's state data 
 */
typedef struct boatVector {
  boatState state;        /**< The current state of the boat                    */
  throttleState throttle;   /**< The current throttle position                    */
  boneState bone;       /**< The current state of the BeagleBone                */
  sensors_vec_t orientation;  /**< The current accelerometer tilt and magnetic heading of the boat  */
  double headingTarget;     /**< The desired magnetic heading                     */  
  double internalVoltage;   /**< The battery voltage measured on the control PCB          */
  double batteryVoltage;    /**< The battery voltage measured at the battery            */
  uint8_t enbButton;      /**< State of the enable button. off = 0; on = 0xff           */
  uint8_t stopButton;     /**< State of the emergency stop button. off = 0; on = 0xff       */
  long timeSinceLastPacket;   /**< Number of milliseconds since the last command packet received    */
} boatVector;

Servo steeringServo;    /**< Servo object corresponding to the steering servo           */
double currentError;    /**< Current heading error. This is a global so the PID can be as well  */
double targetError = 0;   /**< Desired heading error. This is a global so the PID can be as well  */
double steeringCmd;     /**< Steering command to be written out to the servo.           */

/**
 * @brief PID loop object for driving the steering servo
 */
PID steeringPID(&currentError, &steeringCmd, &targetError, Kp, Ki, Kd, REVERSE); 
Adafruit_9DOF dof =           Adafruit_9DOF();            /**< IMU object               */
Adafruit_LSM303_Accel_Unified accel =   Adafruit_LSM303_Accel_Unified(30301); /**< Accelerometer object           */
Adafruit_LSM303_Mag_Unified   mag   =   Adafruit_LSM303_Mag_Unified(30302);   /**< Magnetometer object          */
boatVector boat;                                /**< Boat state vector            */
const double emergencyHeading = 90;                       /**< Emergency heading for self-recovery  */
uint16_t faultString = 0;                           /**< Fault string -- binary string      */

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
boatState executeArmedTest(boatVector * thisBoat, boatState lastState, stateCmd cmd);
boatState executeActiveRudder(boatVector * thisBoat, boatState lastState, stateCmd cmd);
void lightControl(boatState state, boneState bone);
int output (throttleState * throttle, double error);
int writeMavlinkPackets (boatVector * thisBoat, double batCurrent, double motVoltage, double motCurrent, long * lastPacketOut);
int writeNVM (boatState state, throttleState throttle, double heading);
void calibrateMag (sensors_event_t *magEvent);
void calibrateAccel (sensors_event_t *accelEvent);

void setup() {
  LogSerial.begin(115200);
  RestSerial.begin(115200);
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

  LogSerial.println("I live!");
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
  steeringPID.SetOutputLimits(pidMin, pidMax);
  steeringServo.attach(steeringPin);
  steeringPID.SetMode(AUTOMATIC);
  LogSerial.println("Everything up!");
  
  boat.throttle = STOP;
  boat.state = BOAT_POWERUP;
}

void loop() {
  // put your main code here, to run repeatedly:

}
