#include <HackerBoatLib.h>
#include <hardwareConfig.h>
#include <sensorCalibration.h>

aREST     restInput   = aREST();  /**< REST input object **/
boatVector   boat;         /**< Boat state vector            */

void setup() {
  initIO();
  initREST(&restInput, &boat);
  initBoat(&boat);
}

void loop() {
  //Serial.print(".");
  static arduinoState thisState = BOAT_NONE;
  static arduinoState lastState = BOAT_NONE;
  input(&restInput, &boat);
  if (thisState != lastState) {
    Serial.print(F("State change, origin state: "));
    Serial.print(lastState); Serial.print(F(" current state: "));
    Serial.println(thisState);  
  }
  switch(boat.state) {
    case BOAT_POWERUP:
      //Serial.println("Got powerup state");
      thisState = boat.state;
      boat.state = executePowerUp(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_ARMED:
      thisState = boat.state;
      boat.state = executeArmed(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_SELFTEST:
      //Serial.println("Got selftest state");
      thisState = boat.state;
      boat.state = executeSelfTest(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_DISARMED:
      thisState = boat.state;
      boat.state = executeDisarmed(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_ACTIVE:
      thisState = boat.state;
      boat.state = executeActive(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_LOWBATTERY:
      thisState = boat.state;
      boat.state = executeLowBattery(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_FAULT:
      thisState = boat.state;
      boat.state = executeFault(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_SELFRECOVERY:
      thisState = boat.state;
      boat.state = executeSelfRecovery(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_ARMEDTEST:
      thisState = boat.state;
      boat.state = executeArmedTest(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_ACTIVERUDDER:
      thisState = boat.state;
      boat.state = executeActiveRudder(&boat, lastState);
      lastState = thisState;
      break;
    case BOAT_NONE:
    default:
      LogSerial.print(F("Got bad arduino state value: ")); 
      boat.state = BOAT_SELFTEST;
      lastState = BOAT_NONE;
      thisState = BOAT_NONE;
      break;
  }
  output(&boat);
}
