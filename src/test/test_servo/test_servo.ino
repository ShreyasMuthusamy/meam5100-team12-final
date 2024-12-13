#include <ESP32Servo.h>

const int servoPin = 9;
Servo myServo;

void setup() {
  // put your setup code here, to run once:
  myServo.attach(servoPin, 500, 2500);
}

void loop() {
  // put your main code here, to run repeatedly:
  myServo.write(0);
  delay(1000);
  myServo.write(180);
  delay(1000);
}
