#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>
#include "mavlink.h"

const uint8_t servoEnable =    2;
const uint8_t steeringPin =    3;
const uint8_t batVolt =        A0;
const uint8_t relayDir =       52;
const uint8_t relayDirFB =     53;
const uint8_t relaySpeed1 =    51;
const uint8_t relaySpeed1FB =  50;
const uint8_t relaySpeed3 =    48;  
const uint8_t relaySpeed3FB =  49;
const uint8_t relaySpeed5 =    47;  
const uint8_t relaySpeed5FB =  46;
const uint16_t sendDelay =     1000;
const uint32_t lightTimeout =  1490000;
const uint32_t ctrlTimeout =   1500000;
const uint16_t lowVoltCutoff = 750;
//const uint8_t relayAux1 =      47;

Servo steeringServo;

double currentError;
double targetError = 0;
double steeringCmd;
const double emergencyHeading = 90;
double targetHeading;
int16_t throttle = 0;

PID steeringPID(&currentError, &steeringCmd, &targetError, 1, 0.01, 0, REVERSE); // changed in response to test results 2014Jul12-1750
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

double getHeadingError (double heading, double headingSet);

typedef enum BOAT_STATE {
  SAFE,
  ARMED,
  LOW_VOLTAGE,
  PANIC
} BOAT_STATE;

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
  
  targetHeading = emergencyHeading;
  Serial.println("I live!");
  Wire.begin();
  
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
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
}

void loop (void) {
  static BOAT_STATE state = SAFE;
  static mavlink_message_t msg;
  static mavlink_status_t stat;
  static long lastShoreTime = millis();
  static long lastCtrlTime = millis();
  static long lastPacketOut = 0;
  static uint16_t shorePtr = 0;
  static uint16_t ctrlPtr = 0;
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t   orientation;
  
  //Serial.println("Reading sensors...");
  accel.getEvent(&accel_event);
  //Serial.println("Accel read");
  //Serial.flush();
  mag.getEvent(&mag_event);
  //Serial.println("Mag read");
  //Serial.flush();
  dof.accelGetOrientation(&accel_event, &orientation);
  dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation);
  currentError = getHeadingError(orientation.heading, targetHeading);
  //Serial.println("Sensors processed");
  //Serial.flush();

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
    case SAFE:
      state = ARMED;
      break;
    case ARMED:
      if (analogRead(batVolt) < lowVoltCutoff) {
        state = LOW_VOLTAGE;
        Serial.print("Activating low voltage lockout: ");
        Serial.println(analogRead(batVolt));
        break;
      }
      if ((millis() - lastCtrlTime) > ctrlTimeout) {
        state = PANIC;
        Serial.println("Fail-safe activated");
        break;
      }
      switch (throttle) {
        case 1:
          digitalWrite(relayDir, LOW);
          digitalWrite(relaySpeed1, HIGH);
        case 0:
          digitalWrite(relayDir, LOW);
          digitalWrite(relaySpeed1, LOW);
        default:
          digitalWrite(relayDir, LOW);
          digitalWrite(relaySpeed1, LOW);
      }
      break;
    case LOW_VOLTAGE:
      if (analogRead(batVolt) > lowVoltCutoff) {
        state = ARMED;
        break;
      }
      digitalWrite(servoEnable, LOW);
      digitalWrite(relayDir, LOW);
      digitalWrite(relaySpeed1, LOW);
      digitalWrite(relaySpeed3, LOW);
      digitalWrite(relaySpeed5, LOW);
      break;
    case PANIC:
      if (analogRead(batVolt) < lowVoltCutoff) {
        state = LOW_VOLTAGE;
        Serial.print("Activating low voltage lockout: ");
        Serial.println(analogRead(batVolt));
        break;
      }
      if ((millis() - lastCtrlTime) < ctrlTimeout) {
        state = ARMED;
        Serial.println("Contact re-established");
        break;
      }
      targetHeading = emergencyHeading;
      digitalWrite(relayDir, LOW);
      digitalWrite(relaySpeed1, HIGH);
      break;
    default:
      state = SAFE;
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
  
}

double getHeadingError (double heading, double headingSet) {
  double result = headingSet - heading;
  
  if (result > 180) {result -= 360;}
  if (result < -180) {result += 360;}
  
  return result;
}

