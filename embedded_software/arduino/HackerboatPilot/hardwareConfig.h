// steering and servo constants
const double Kp =               5.0;
const double Ki =               0.25;
const double Kd =               0.0;
const double pidMax =           100.0;
const double pidMin =           -100.0;
const uint16_t servoMin =       860;
const uint16_t servoMax =       2240;

// test limits
const double compassDeviationLimit = 	10.0;	/**< Limit of compass swing, in degrees, during test period 	*/
const double tiltDeviationLimit =    	15.0;	/**< Limit of tilt in degrees from horizontal during test		*/
const double rateDeviationLimit =    	15.0;	/**< Gyro rate limit, in degrees. Currently unused. 			*/
const double testVoltageLimit =     	12.0;	/**< Battery voltage lower limit during powerup test			*/
const double serviceVoltageLimit =   	10.0;	/**< Battery voltage lower limit in service						*/
const double recoverVoltageLimit = 		11.0;	/**< Battery voltage required to recover from low battery state	*/

// time delays
const int32_t sensorTestPeriod = 	    5000;	/**< Period to check for sensor deviations, in ms 							*/
const int32_t signalTestPeriod =     	6000;	/**< Period to wait for Beaglebone signal 									*/
const int32_t startupTestPeriod =    	6500;	/**< Period to stay in the self-test state 									*/
const int32_t enbButtonTime =       	5000;	/**< Time the enable button needs to be pressed, in ms, to arm the boat		*/
const int32_t stopButtonTime =      	250;	/**< Time the stop button needs to be pressed, in ms, to disarm the boat	*/
const int32_t disarmedPacketTimeout = 	60000;	/**< Connection timeout, in ms, in the disarmed state						*/
const int32_t armedPacketTimeout =  	60000;	/**< Connection timeout, in ms, in the armed state							*/
const int32_t activePacketTimeout = 	300000;	/**< Connection timeout, in ms, in the active state							*/
const int32_t hornTimeout = 			2000;	/**< Time in ms to sound the horn for before entering an unsafe state		*/	
const int16_t sendDelay =           	1000;	/**< Time in ms between packet transmissions 								*/
const int16_t flashDelay = 				500;	/**< Time in ms between light transitions while flashing					*/

// port mapping
HardwareSerial LogSerial = 				Serial;		/**< Serial port used for logging and feedback */
HardwareSerial RestSerial = 			Serial1;	/**< Serial port used for REST commands and response */

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
const uint32_t grn = Adafruit_NeoPixel::Color(0, 0xff, 0);			/**< pixel colors for green		*/
const uint32_t red = Adafruit_NeoPixel::Color(0xff, 0, 0);			/**< pixel colors for red		*/
const uint32_t blu = Adafruit_NeoPixel::Color(0, 0, 0xff);			/**< pixel colors for blue		*/
const uint32_t amb = Adafruit_NeoPixel::Color(0xff, 0xbf, 0);		/**< pixel colors for amber		*/
const uint32_t wht = Adafruit_NeoPixel::Color(0xff, 0xff, 0xff);	/**< pixel colors for white		*/

//const uint32_t lightTimeout =        1490000;
//const uint32_t ctrlTimeout =         1500000;
//const uint16_t lowVoltCutoff =       750;
//const uint8_t relayAux1 =      47;
