#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <PID_v1.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_NeoPixel.h>

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
const uint32_t grn = Adafruit_NeoPixel::Color(0, 0xff, 0);			/**< pixel colors for green			*/
const uint32_t red = Adafruit_NeoPixel::Color(0xff, 0, 0);			/**< pixel colors for red			*/
const uint32_t blu = Adafruit_NeoPixel::Color(0, 0, 0xff);			/**< pixel colors for blue			*/
const uint32_t amb = Adafruit_NeoPixel::Color(0xff, 0xbf, 0);		/**< pixel colors for amber			*/
const uint32_t wht = Adafruit_NeoPixel::Color(0xff, 0xff, 0xff);	/**< pixel colors for white			*/

Adafruit_9DOF dof = 					Adafruit_9DOF(); 						/**< IMU object 										*/
Adafruit_LSM303_Accel_Unified accel = 	Adafruit_LSM303_Accel_Unified(30301);	/**< Accelerometer object 								*/
Adafruit_LSM303_Mag_Unified   mag   = 	Adafruit_LSM303_Mag_Unified(30302);		/**< Magnetometer object 								*/
Servo steeringServo;															/**< Servo object corresponding to the steering servo 	*/
Adafruit_NeoPixel ardLights = Adafruit_NeoPixel(ardLightCount, arduinoLightsPin,  NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boneLights = Adafruit_NeoPixel(boneLightCount, boneLightsPin,  NEO_GRB + NEO_KHZ800);
  
void printInputs (void);
void stopAllOutputs (void);
void StopAllLights (void);
void cycleLights (void);
void ardWhiteLights (void);
void boneWhiteLights (void);
void setLights (uint32_t ardColor, uint32_t boneColor);
void servoDrive (void);

void setup() {
  // put your setup code here, to run once:
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
  steeringServo.attach(steeringPin);	
  ardLights.begin();
  boneLights.begin();

}

void loop() {
  char c;
  // put your main code here, to run repeatedly:
  if(Serial.available()) c = Serial.read();
  
  switch (c) {
	case 'R':	// read the data values 
	  printInputs();
	  break;
	case '1':	// activate reverse relay
	  stopAllOutputs();
	  digitalWrite(relayDir, HIGH);
	  break;
	case '2':	// activate red relay
	  stopAllOutputs();
	  digitalWrite(relaySpeedRed, HIGH);
	  break;
	case '3':	// activate yellow relay
	  stopAllOutputs();
	  digitalWrite(relaySpeedYlw, HIGH);
	  break;
	case '4':	// activate white relay
	  stopAllOutputs();
	  digitalWrite(relaySpeedWht, HIGH);
	  break;
	case '5':	// activate red-white relay
	  stopAllOutputs();
	  digitalWrite(relaySpeedRedWht, HIGH);
	  break;
	case '6':	// activate red-yellow relay
	  stopAllOutputs();
	  digitalWrite(relaySpeedRedYlw, HIGH);
	  break;
	case 'h':	// turn the horn on
	  stopAllOutputs();
	  digitalWrite(horn, HIGH);
	  break;
	case 's':	// stop all outputs
	  stopAllOutputs();
	  break;
	case 'l':	// turn all lights off
	  stopAllLights();
	  break;
	case 'L': 	// cycle the lights through all colors
	  cycleLights();
	  break;
	case 'w':	// turn the arduino lights white
	  ardWhiteLights();
	  break;
	case 'W': 	// turn the beaglebone lights white
	  boneWhiteLights();
	  break;
	case 'S':	// manipulate the servo output
	  servoDrive();
	  break;
	default:
	  stopAllOutputs();
	  break;
  }

}

void printInputs (void) {
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t orientation;
   
  // get & process the IMU data
  accel.getEvent(&accel_event);
  mag.getEvent(&mag_event);
  dof.accelGetOrientation(&accel_event, &orientation);
  dof.magTiltCompensation(SENSOR_AXIS_Z, &mag_event, &accel_event);
  dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation);
  
  Serial.print("Heading:\t\t"); Serial.println(orientation.heading);
  Serial.print("Roll:\t\t"); Serial.println(orientation.roll);
  Serial.print("Pitch:\t\t"); Serial.println(orientation.pitch);
  Serial.print("Internal Voltage:\t"); Serial.println(analogRead(internalBatVolt));
  Serial.print("Motor Voltage:\t\t"); Serial.println(analogRead(motorVolt));
  Serial.print("Motor Current:\t\t"); Serial.println(analogRead(motorCurrent));
  Serial.print("Direction Relay:\t"); Serial.println(digitalRead(relayDirFB));
  Serial.print("White Relay:\t"); Serial.println(digitalRead(relaySpeedWhtFB));
  Serial.print("Red Relay:\t\t"); Serial.println(digitalRead(relaySpeedRedFB));
  Serial.print("Yellow Relay:\t"); Serial.println(digitalRead(relaySpeedYlwFB));
  Serial.print("Red/Wht Relay:\t\t"); Serial.println(digitalRead(relaySpeedRedWhtFB));
  Serial.print("Red/Ylw Relay:\t"); Serial.println(digitalRead(relaySpeedRedYlwFB));
  Serial.print("Horn:\t\t"); Serial.println(digitalRead(hornFB));
  Serial.print("Enable Button:\t"); Serial.println(digitalRead(enableButton));
  Serial.print("Stop Button:\t"); Serial.println(digitalRead(stopButton));
}

void stopAllOutputs (void) {
  digitalWrite(servoEnable, LOW);
  digitalWrite(relayDir, LOW);
  digitalWrite(relaySpeedWht, LOW);
  digitalWrite(relaySpeedYlw, LOW);
  digitalWrite(relaySpeedRed, LOW);
  digitalWrite(relaySpeedRedWht, LOW);
  digitalWrite(relaySpeedRedYlw, LOW);
  digitalWrite(horn, LOW);
  stopAllLights();
}

void stopAllLights (void) {
  setLights(0,0);
}

void cycleLights (void) {
  setLights(red, grn);
  delay(500);
  setLights(blu, red);
  delay(500);
  setLights(amb, blu);
  delay(500);
  setLights(wht, amb);
  delay(500);
  setLights(grn, wht);
  delay(500);
  setLights(0,0);	
}

void ardWhiteLights (void) {
  setLights(wht,0);
}

void boneWhiteLights (void) {
  setLights(0, wht);
}

void setLights (uint32_t ardColor, uint32_t boneColor) {
  uint8_t i;
  
  for (i = 0; i < ardLightCount; i++) {
    ardLights.setPixelColor(i, ardColor);
  }
  
  for (i = 0; i < boneLightCount; i++) {
    boneLights.setPixelColor(i, boneColor);
  }
  
  ardLights.show();
  boneLights.show();
}

void servoDrive (void) {
  char c;
  
  digitalWrite(servoEnable, HIGH);
  if (Serial.available()) {
    c = Serial.read();
	switch (c) {
	  case '0':
	    steeringServo.write(0);
	    break;
	  case '1':
	    steeringServo.write(20);
	    break;
	  case '2':
	    steeringServo.write(40);
	    break;
	  case '3':
	    steeringServo.write(60);
	    break;
	  case '4':
	    steeringServo.write(80);
	    break;
	  case '5':
	    steeringServo.write(100);
	    break;
	  case '6':
	    steeringServo.write(120);
	    break;
	  case '7':
	    steeringServo.write(140);
	    break;
	  case '8':
	    steeringServo.write(160);
	    break;
	  case '9':
	    steeringServo.write(180);
	    break;
	  default:
	    break;
	}
  }
	
}
