#include <HackerBoatLib.h>
#include <hardwareConfig.h>
#include <sensorCalibration.h>

aREST     restInput   = aREST();  /**< REST input object **/
boatVector   boat;         /**< Boat state vector            */

void setup() {
  initIO();
  initREST(&restInput, &boat);
  initBoat(&boat);
  Serial.print(F("Time,Mode,Heading Target,Heading,Throttle,Rudder (Raw),"));
  Serial.println(F("Internal Voltage (Raw),Motor Voltage (Raw), Motor Current (Raw)"));
}

void loop() {
  static long last = millis();
  static arduinoMode thisMode = ARD_NONE;
  static arduinoMode lastMode = ARD_NONE;
  while (millis() < (last + 100)) {
   input(&restInput, &boat);
  }
  last = millis();
  if (thisMode != lastMode) {
    //Serial.print(F("State change, origin state: "));
    //Serial.print(lastMode); Serial.print(F(" current state: "));
    //Serial.println(lastMode);  
  }
  switch(boat.mode) {
    case ARD_POWERUP:
      //Serial.println("Got powerup state");
      thisMode = boat.mode;
      boat.mode = executePowerUp(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_ARMED:
      thisMode = boat.mode;
      boat.mode = executeArmed(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_SELFTEST:
      //Serial.println("Got selftest state");
      thisMode = boat.mode;
      boat.mode = executeSelfTest(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_DISARMED:
      thisMode = boat.mode;
      boat.mode = executeDisarmed(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_ACTIVE:
      thisMode = boat.mode;
      boat.mode = executeActive(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_LOWBATTERY:
      thisMode = boat.mode;
      boat.mode = executeLowBattery(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_FAULT:
      thisMode = boat.mode;
      boat.mode = executeFault(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_SELFRECOVERY:
      thisMode = boat.mode;
      boat.mode = executeSelfRecovery(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_ARMEDTEST:
      thisMode = boat.mode;
      boat.mode = executeArmedTest(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_ACTIVERUDDER:
      thisMode = boat.mode;
      boat.mode = executeActiveRudder(&boat, lastMode);
      lastMode = thisMode;
      break;
    case ARD_NONE:
    default:
      LogSerial.print(F("Got bad arduino mode value: ")); 
      boat.mode = ARD_SELFTEST;
      lastMode = ARD_NONE;
      thisMode = ARD_NONE;
      break;
  }
  output(&boat);
}
