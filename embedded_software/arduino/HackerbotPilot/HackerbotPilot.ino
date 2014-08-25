#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include "mavlink.h"

#define FAULT_LOW_BAT          0x0001
#define FAULT_SENSOR           0x0002
#define FAULT_NO_SIGNAL        0x0004
#define FAULT_BB_FAULT         0x0008
#define FAULT_NVM              0x0010

const int32_t sensorTestPeriod =     5000;
const int32_t signalTestPeriod =     60000;
const int32_t startupTestPeriod =    65000;
const double compassDeviationLimit = 10.0;
const double tiltDeviationLimit =    15.0;
const double rateDeviationLimit =    15.0;
const double testVoltageLimit =      12.0;
const double serviceVoltageLimit =   10.0;

const uint32_t enbButtonTime =       10000;
const uint32_t stopButtonTime =      1000;
const uint32_t disarmedPacketTimeout = 60000;
const uint32_t armedPacketTimeout =  60000;
const uint32_t activePacketTimeout = 300000;

const uint8_t servoEnable =          2;
const uint8_t steeringPin =          3;
const uint8_t internalBatVolt =      A0;
const double internalBatVoltMult =   1.0;
const uint8_t relayDir =             52;
const uint8_t relayDirFB =           53;
const uint8_t relaySpeed1 =          51;
const uint8_t relaySpeed1FB =        50;
const uint8_t relaySpeed3 =          48;  
const uint8_t relaySpeed3FB =        49;
const uint8_t relaySpeed5 =          47;  
const uint8_t relaySpeed5FB =        46;
const uint16_t sendDelay =           1000;
const uint32_t lightTimeout =        1490000;
const uint32_t ctrlTimeout =         1500000;
const uint16_t lowVoltCutoff =       750;
//const uint8_t relayAux1 =      47;

/*! \var typedef enum boatState
 *  \brief An enum to store the current state of the boat.
 */
typedef enum boatState {
  BOAT_POWERUP,
  BOAT_ARMED,
  BOAT_SELFTEST,
  BOAT_DISARMED,
  BOAT_ACTIVE,
  BOAT_LOWBATTERY,
  BOAT_FAULT,
  BOAT_SELFRECOVERY
} boatState;

typedef enum throttleState {
  FWD5,
  FWD4,
  FWD3,
  FWD2,
  FWD1,
  STOP,
  REV1,
  REV2,
  REV3
} throttleState;

typedef enum stateCmd {
  CMD_NONE,
  CMD_ARM,   
  CMD_DISARM,
  CMD_ACTIVE,
  CMD_TEST,
  CMD_FAULT        // If the Beaglebone declares a fault state, the 
} stateCmd;

typedef struct boatVector {
  boatState state;
  throttleState throttle;
  sensors_vec_t orientation;
  double headingTarget;
  double internalVoltage;
  double batteryVoltage;
  uint8_t enbButton;
  uint8_t stopButton;
  long timeSinceLastPacket;
} boatVector;

Servo steeringServo;
double currentError;
double targetError = 0;
double steeringCmd;
PID steeringPID(&currentError, &steeringCmd, &targetError, 1, 0.01, 0, REVERSE); // changed in response to test results 2014Jul12-1750
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
boatVector boat;
const double emergencyHeading = 90;
uint16_t faultString = 0;

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

/*! \fn double getHeadingError (double heading, double headingSet)
 *  \brief generate heading error from current heading and desired heading
 *
 *  \param heading
 *  \param headingSet
 *  \return
 */
 
double getHeadingError (double heading, double headingSet) {
  double result = headingSet - heading;
  
  if (result > 180) {result -= 360;}
  if (result < -180) {result += 360;}
  
  return result;
}


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

long int getPackets (boatVector * thisBoat, stateCmd * cmd) {
  static mavlink_message_t msg;
  static mavlink_status_t stat;
  static long lastCtrlTime = millis();
  
  
  return 0;
}

int getNVM (boatState * state, throttleState * throttle, double * heading) {
  return 0;
}

/*! \fn int executeSelfTest (void)
 *  \brief Run internal self tests to confirm that the boat's systems are working correctly.
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
  double intVoltage;
  double batVoltage;
  double batCurrent;
  double motVoltage;
  double motCurrent;
  uint8_t enbButton;
  uint8_t estopButton;
  
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


