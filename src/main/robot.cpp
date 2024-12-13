#include "robot.h"

// Need this code for interrupts to work in OOP setting
Robot* Robot::robot = nullptr;
void IRAM_ATTR Robot::handleLeftEncoderInterrupt() { if (robot) { robot->updateLeftEncoder(); } }
void IRAM_ATTR Robot::handleRightEncoderInterrupt() { if (robot) { robot->updateRightEncoder(); } }

// Setup the motors using the ledc and digital pinMode functions
void Robot::setupMotors() {
  ledcAttach(leftMotorPWM, MOTOR_FREQ, MOTOR_RES_BITS);
  ledcAttach(rightMotorPWM, MOTOR_FREQ, MOTOR_RES_BITS);
  pinMode(leftMotorDir, OUTPUT);
  pinMode(rightMotorDir, OUTPUT);
  servo.attach(servoSig);
}

// Setup the encoders with INPUT_PULLUP and to use the interrupts defined in robot.h
void Robot::setupEncoders() {
  pinMode(leftEncoderA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(leftEncoderA), handleLeftEncoderInterrupt, RISING);
  pinMode(leftEncoderB, INPUT_PULLUP);
  pinMode(rightEncoderA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rightEncoderA), handleRightEncoderInterrupt, RISING);
  pinMode(rightEncoderB, INPUT_PULLUP);
}

// Set up the IR sensors (Vive sensors are setup in vive510.cpp)
void Robot::setupSensors() {
  pinMode(leftIRShut, OUTPUT);
  pinMode(frontIRShut, OUTPUT);
  pinMode(rightIRShut, OUTPUT);

  digitalWrite(leftIRShut, LOW);
  digitalWrite(frontIRShut, LOW);
  digitalWrite(rightIRShut, LOW);
  delay(10);

  digitalWrite(leftIRShut, HIGH);
  digitalWrite(frontIRShut, HIGH);
  digitalWrite(rightIRShut, HIGH);
  delay(10);

  digitalWrite(leftIRShut, HIGH);
  digitalWrite(frontIRShut, LOW);
  digitalWrite(rightIRShut, LOW);
  if(!leftIR.begin(leftIRAddress, false, &Wire)) {
    Serial.println(F("Failed to boot left VL53L0X"));
    while(1);
  }
  delay(10);

  digitalWrite(frontIRShut, HIGH);
  if(!frontIR.begin(frontIRAddress, false, &Wire)) {
    Serial.println(F("Failed to boot front VL53L0X"));
    while(1);
  }
  delay(10);

  digitalWrite(rightIRShut, HIGH);
  if(!rightIR.begin(rightIRAddress, false, &Wire)) {
    Serial.println(F("Failed to boot right VL53L0X"));
    while(1);
  }

  Serial.println("Setup sensors (let's gooooo)");
}

int Robot::solvePIDLeft(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;
  
  // Integral error
  static float errIL = 0.0;
  errIL += (float) err / FRAME_RATE;
  errIL = constrain(errIL, -MAX_ERR_I/kIL, MAX_ERR_I/kIL);
  
  // Derivative error
  static int oldValL;
  int errD = (current - oldValL) * FRAME_RATE;

  // Calculate PID control action
  int u = kPL * err + kIL * errIL - kDL * errD;

  oldValL = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
}

int Robot::solvePIDRight(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;

  // Integral error
  static float errIR = 0.0;
  errIR += (float) err / FRAME_RATE;
  errIR = constrain(errIR, -MAX_ERR_I/kIR, MAX_ERR_I/kIR);
  
  // Derivative error
  static int oldValR;
  int errD = (current - oldValR) * FRAME_RATE;
  
  // Calculate PID control action
  int u = kPR * err + kIR * errIR - kDR * errD;

  oldValR = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
}

int Robot::getDistance(Adafruit_VL53L0X &tof) {
  VL53L0X_RangingMeasurementData_t measure;
  tof.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter;
  } else {
    return -1;
  }
}

// Initialize the robot and setup motors, encoders, and sensors
void Robot::init() {
  Wire.begin(SDA_PIN, SCL_PIN);

  setupMotors();
  setupEncoders();
  setupSensors();
}

void Robot::update() {
  static int prevLeftEncoderCounts = 0;
  static int prevRightEncoderCounts = 0;

  vLeft = leftEncoderCounts - prevLeftEncoderCounts;
  vRight = rightEncoderCounts - prevRightEncoderCounts;

  // Serial.printf("Actual Velocity: %d, %d\n", vLeft, vRight);

  prevLeftEncoderCounts = leftEncoderCounts;
  prevRightEncoderCounts = rightEncoderCounts;
}

int Robot::getLeftDistance() {
  static float reading = 0;
  reading += (getDistance(leftIR) - reading) / 4;
  return reading;
}

int Robot::getFrontDistance() {
  static float reading = 0;
  reading += (getDistance(frontIR) - reading) / 4;
  return reading;
}

int Robot::getRightDistance() {
  static float reading = 0;
  reading += (getDistance(rightIR) - reading) / 4;
  return reading;
}

void Robot::drive(int left, int right) {
  left = constrain(left, -40, 40);
  right = constrain(right, -40, 40);

  int uLeft = solvePIDLeft(vLeft, left);  // Solve PID to get control input
  leftFwd = uLeft > 0;      // Set the variable that will control the positive or negative increment of the encoders
  digitalWrite(leftMotorDir, !leftFwd);    // The inverters set the necessary voltages for the H-bridge, so only one pin is needed
  ledcWrite(leftMotorPWM, map(abs(uLeft), 0, 100, 0, MOTOR_RES));   // Map the duty cycle percentage to the motor resolution

  int uRight = solvePIDRight(vRight, right);
  rightFwd = uRight > 0;
  digitalWrite(rightMotorDir, !rightFwd);
  ledcWrite(rightMotorPWM, map(abs(uRight), 0, 100, 0, MOTOR_RES));
}

void Robot::fullSend(int left, int right) {
  leftFwd = left > 0;      // Set the variable that will control the positive or negative increment of the encoders
  digitalWrite(leftMotorDir, !leftFwd);    // The inverters set the necessary voltages for the H-bridge, so only one pin is needed
  ledcWrite(leftMotorPWM, map(abs(left), 0, 100, 0, MOTOR_RES));   // Map the duty cycle percentage to the motor resolution

  rightFwd = right > 0;
  digitalWrite(rightMotorDir, !rightFwd);
  ledcWrite(rightMotorPWM, map(abs(right), 0, 100, 0, MOTOR_RES));
}

void Robot::attack() {
  servo.write(180);
}
