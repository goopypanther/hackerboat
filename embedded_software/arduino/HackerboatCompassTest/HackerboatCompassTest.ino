#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

/* Assign a unique ID to these sensors */
Adafruit_LSM303_Accel_Unified   accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified     mag   = Adafruit_LSM303_Mag_Unified(12345);
long lastDisplayTime;

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

  if ((millis() - lastDisplayTime) > 100) {
    // get & process the IMU data
    accel.getEvent(&accel_event);
    mag.getEvent(&mag_event);
    calibrateMag(&mag_event);
    calibrateAccel(&accel_event);
    dof.accelGetOrientation(&accel_event, &orientation);
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
    Serial.print(orientation.heading);
    
  }
}
