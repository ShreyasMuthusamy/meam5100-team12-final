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

int Robot::solvePIDLeft(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;
  
  // Integral error
  static float errIL = 0.0;
  errIL += (float) err / FRAME_RATE;
  errIL = constrain(errIL, -MAX_ERR_I/KIL, MAX_ERR_I/KIL);
  
  // Derivative error
  static int oldValL;
  int errD = (current - oldValL) * FRAME_RATE;

  // Calculate PID control action
  int u = kP * err + kI * errIL - kD * errD;

  oldValL = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
}

int Robot::solvePIDRight(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;

  // Integral error
  static float errIR = 0.0;
  errIR += (float) err / FRAME_RATE;
  errIR = constrain(errIR, -MAX_ERR_I/KIR, MAX_ERR_I/KIR);
  
  // Derivative error
  static int oldValR;
  int errD = (current - oldValR) * FRAME_RATE;
  
  // Calculate PID control action
  int u = kP * err + kI * errIR - kD * errD;

  oldValR = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
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
  int uLeft = solvePIDLeft(vLeft, left);  // Solve PID to get control input
  leftFwd = uLeft > 0;      // Set the variable that will control the positive or negative increment of the encoders
  digitalWrite(leftMotorDir, leftFwd);    // The inverters set the necessary voltages for the H-bridge, so only one pin is needed
  ledcWrite(leftMotorPWM, map(abs(uLeft), 0, 100, 0, MOTOR_RES));   // Map the duty cycle percentage to the motor resolution

  int uRight = solvePIDRight(vRight, right);
  rightFwd = uRight > 0;
  digitalWrite(rightMotorDir, rightFwd);
  ledcWrite(rightMotorPWM, map(abs(uRight), 0, 100, 0, MOTOR_RES));
}
