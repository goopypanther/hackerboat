#include <HackerBoatLib.h>
#include <hardwareConfig.h>
#include <sensorCalibration.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("float size: "); Serial.println(sizeof(float));
  Serial.print("long size: "); Serial.println(sizeof(long));
  Serial.print("String size: "); Serial.println(sizeof(String));
  Serial.print("Servo size: "); Serial.println(sizeof(Servo));
  Serial.print("PID size: "); Serial.println(sizeof(PID));
  Serial.print("9dof size: "); Serial.println(sizeof(Adafruit_9DOF));
  Serial.print("accel size: "); Serial.println(sizeof(Adafruit_LSM303_Accel_Unified));
  Serial.print("mag size: "); Serial.println(sizeof(Adafruit_LSM303_Mag_Unified));
  Serial.print("gyro size: "); Serial.println(sizeof(Adafruit_L3GD20_Unified));
  Serial.print("light size: "); Serial.println(sizeof(Adafruit_NeoPixel));
  Serial.print("boatVector size: "); Serial.println(sizeof(boatVector));
  Serial.print("aREST size: "); Serial.println(sizeof(aREST));
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
