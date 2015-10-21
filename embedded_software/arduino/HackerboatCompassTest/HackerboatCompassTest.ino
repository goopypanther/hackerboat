#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

/* Assign a unique ID to these sensors */
Adafruit_LSM303_Accel_Unified   accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified     mag   = Adafruit_LSM303_Mag_Unified(12345); 
Adafruit_9DOF dof =          Adafruit_9DOF();            /**< IMU object               */
long lastDisplayTime;

// calibration constants
// these constants are used to calibrate the accelerometer and magnetometer
#define X_ACCEL_OFFSET          (0)
#define X_ACCEL_GAIN            (0.1)
#define Y_ACCEL_OFFSET          (0)
#define Y_ACCEL_GAIN            (0.1)
#define Z_ACCEL_OFFSET          (0)
#define Z_ACCEL_GAIN            (0.1)
#define X_MAG_OFFSET            (-39.59)
#define X_MAG_GAIN              (0.933)
#define Y_MAG_OFFSET            (10.82)
#define Y_MAG_GAIN              (0.943)
#define Z_MAG_OFFSET            (-23.16)
#define Z_MAG_GAIN              (1.054)

void calibrateMag (sensors_event_t *magEvent) {
  float *mag_X, *mag_Y, *mag_Z;
  mag_X = &(magEvent->magnetic.x);
  mag_Y = &(magEvent->magnetic.y);
  mag_Z = &(magEvent->magnetic.z);
  *mag_X += X_MAG_OFFSET;
  *mag_X *= X_MAG_GAIN;
  *mag_Y += Y_MAG_OFFSET;
  *mag_Y *= Y_MAG_GAIN;
  *mag_Z += Z_MAG_OFFSET;
  *mag_Z *= Z_MAG_GAIN;
}

void calibrateAccel (sensors_event_t *accelEvent) {
  float *accel_X, *accel_Y, *accel_Z;
  accel_X = &(accelEvent->acceleration.x);
  accel_Y = &(accelEvent->acceleration.y);
  accel_Z = &(accelEvent->acceleration.z);
  *accel_X += X_ACCEL_OFFSET;
  *accel_X *= X_ACCEL_GAIN;
  *accel_Y += Y_ACCEL_OFFSET;
  *accel_Y *= Y_ACCEL_GAIN;
  *accel_Z += Z_ACCEL_OFFSET;
  *accel_Z *= Z_ACCEL_GAIN;
}

void setup() {
  
  Serial.begin(115200);
  Serial.println("LSM303 Testing"); Serial.println("");
  
  /* Initialise the accelerometer */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  /* Initialise the magnetometer */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  lastDisplayTime = millis();
}

void loop() {  
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t orientation;
  static uint8_t count = 0;
  double heading;

  if ((millis() - lastDisplayTime) > 100) {
    // get & process the IMU data
    accel.getEvent(&accel_event);
    mag.getEvent(&mag_event);
    calibrateMag(&mag_event);
    //calibrateAccel(&accel_event);
    dof.accelGetOrientation(&accel_event, &orientation);
    heading = atan2(mag_event.magnetic.y, mag_event.magnetic.x)*(180/PI);
    if (heading < 0) heading += 360;
    dof.magTiltCompensation(SENSOR_AXIS_Z, &mag_event, &accel_event);
    dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation); 

    // if the count is back to zero, print the header
    if (0 == count) Serial.println ("Roll,\tPitch,\tHeading\t");
    count++;

    // print the data
    Serial.print(orientation.pitch);
    Serial.print(",\t");
    Serial.print(orientation.roll);
    Serial.print(",\t");
    Serial.print(heading);
    Serial.print(",\t");
    Serial.println(orientation.heading);
    
  }
}
