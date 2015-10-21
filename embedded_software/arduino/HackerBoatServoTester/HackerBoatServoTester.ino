#include <Servo.h>

const uint8_t servoEnable = 2;    /**< Enable pin for the steering servo power supply   */
const uint8_t steeringPin = 3;    /**< Steering servo control pin             */
Servo steeringServo;    /**< Servo object corresponding to the steering servo           */

void setup() {
  Serial.begin(115200);
  pinMode(servoEnable, OUTPUT);
  pinMode(steeringPin, OUTPUT);
  steeringServo.attach(steeringPin);
  digitalWrite(servoEnable, HIGH);

  Serial.println("I live!");
  

}

void loop() {
  static uint16_t top = 2000;
  static uint16_t bot = 1000;
  static uint16_t cen = 1500;
  static uint16_t cur = cen;
  static char stat = 'c';
  char c;

  Serial.print("Center: ");
  Serial.print(cen);
  Serial.print(" Top: ");
  Serial.print(top);
  Serial.print(" Bottom: ");
  Serial.print(bot);
  Serial.print(" Current: ");
  Serial.println(cur);
  Serial.println("Press b for bottom, t for top, and c for center, + and - to adjust current setpoint");
  
  while(!Serial.available());

  c = Serial.read();

  switch (c) {
    case 'b':
      Serial.print("Switching to bottom: ");
      Serial.print(bot);
      Serial.println(" us");
      stat = c;
      break;
    case 't':
      Serial.print("Switching to top: ");
      Serial.print(top);
      Serial.println(" us");
      stat = c;
      break;
    case 'c':
      Serial.print("Switching to center: ");
      Serial.print(cen);
      Serial.println(" us");
      stat = c;
      break;
    case '+':
      if (stat == 'c') cen += 10;
      if (stat == 't') top += 10;
      if (stat == 'b') bot += 10;
      break;
    case '-':
      if (stat == 'c') cen -= 10;
      if (stat == 't') top -= 10;
      if (stat == 'b') bot -= 10;
      break;
    default:
      break;
  }

  switch (stat) {
    case 'b':
      cur = bot;
      break;
    case 't':
      cur = top;
      break;
    case 'c':
      cur = cen;
      break;
    default:
      break;
  }
  
  steeringServo.writeMicroseconds(cur);
}
