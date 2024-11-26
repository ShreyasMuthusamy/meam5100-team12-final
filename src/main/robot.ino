#include "robot.h"

// Need this code for interrupts to work in OOP setting
Robot* Robot::robot = nullptr;
void IRAM_ATTR Robot::handleLeftEncoderInterrupt() { if (Robot::robot) { Robot::robot->updateLeftEncoder(); } }
void IRAM_ATTR Robot::handleRightEncoderInterrupt() { if (Robot::robot) { Robot::robot->updateRightEncoder(); } }

// Setup the motors using the ledc and digital pinMode functions
void Robot::setupMotors() {
  ledcAttach(leftMotorPWM, MOTOR_FREQ, MOTOR_RES_BITS);
  ledcAttach(rightMotorPWM, MOTOR_FREQ, MOTOR_RES_BITS);
  pinMode(leftMotorDir, OUTPUT);
  pinMode(rightMotorDir, OUTPUT);
}

// Setup the encoders with INPUT_PULLUP and to use the interrupts defined in robot.h
void Robot::setupEncoders() {
  pinMode(leftEncoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(leftEncoderPin), handleLeftEncoderInterrupt, RISING);
  pinMode(rightEncoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rightEncoderPin), handleRightEncoderInterrupt, RISING);
}

// Initialize the robot and setup motors, encoders, and sensors
void Robot::init() {
  setupMotors();
  setupEncoders();
}

// Update the robot pose
void Robot::update() {
  // do nothing for now
}

void Robot::drive(int left, int right) {
  leftFwd = left > 0;       // Set the variable that will control the positive or negative increment of the encoders
  digitalWrite(MOTOR_L_DIR, leftFwd);   // The inverters set the necessary voltages for the H-bridge, so only one pin is needed
  ledcWrite(MOTOR_L_PWM, map(abs(left), 0, 100, 0, MOTOR_RES));   // Map the duty cycle percentage to the motor resolution

  rightFwd = right > 0;
  digitalWrite(MOTOR_R_DIR, rightFwd);
  ledcWrite(MOTOR_R_PWM, map(abs(right), 0, 100, 0, MOTOR_RES));
}
