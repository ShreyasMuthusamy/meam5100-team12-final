#ifndef ROBOT_H_
#define ROBOT_H_

#include <arduino.h>
#include <ESP32Servo.h>
#include "Adafruit_VL53L0X.h"
#include "geometry.h"
#include "vive510.h"

// Motor LEDC values and PID frame rate
#define MOTOR_RES_BITS 12
#define MOTOR_RES ((1 << MOTOR_RES_BITS) - 1)
#define MOTOR_FREQ 100
#define FRAME_RATE 40

// SDA and SCL pins for I2C
#define SDA_PIN 41
#define SCL_PIN 42

// Set a cap on the maximum integral error
#define MAX_ERR_I 100

class Robot {
  private:
    static Robot* robot;

    int leftMotorPWM, leftMotorDir, leftEncoderA, leftEncoderB;
    int rightMotorPWM, rightMotorDir, rightEncoderA, rightEncoderB;
    int leftIRAddress, frontIRAddress, rightIRAddress;
    int leftIRShut, frontIRShut, rightIRShut;
    Servo servo;
    Vive510 leftVive, rightVive;

    Adafruit_VL53L0X leftIR = Adafruit_VL53L0X();
    Adafruit_VL53L0X frontIR = Adafruit_VL53L0X();
    Adafruit_VL53L0X rightIR = Adafruit_VL53L0X();

    long leftEncoderCounts, rightEncoderCounts;
    int servoAngle = 0;
    bool leftFwd, rightFwd, servoCW;
    
    void updateLeftEncoder() { leftEncoderCounts += digitalRead(leftEncoderB) ? 1 : -1; }
    void updateRightEncoder() { rightEncoderCounts += !digitalRead(rightEncoderB) ? 1 : -1; }

    static void IRAM_ATTR handleLeftEncoderInterrupt();
    static void IRAM_ATTR handleRightEncoderInterrupt();

    float kPL, kIL, kDL;
    float kPR, kIR, kDR;
    int vLeft, vRight;

    int solvePIDLeft(int current, int setpoint);
    int solvePIDRight(int current, int setpoint);

    uint16_t getDistance(Adafruit_VL53L0X tof);

    void setupMotors();
    void setupEncoders();
    void setupSensors();

  public:
    Robot(
      int PWML, int DIRL, int ENCLA, int ENCLB,
      int PWMR, int DIRR, int ENCRA, int ENCRB,
      int IRLAdd, int IRFAdd, int IRRAdd,
      int IRLShut, int IRFShut, int IRRShut,
      int viveL, int viveR, int servoPin
    ):
        leftMotorPWM(PWML), leftMotorDir(DIRL), leftEncoderA(ENCLA), leftEncoderB(ENCLB),
        rightMotorPWM(PWMR), rightMotorDir(DIRR), rightEncoderA(ENCRA), rightEncoderB(ENCRB),
        leftIRAddress(IRLAdd), frontIRAddress(IRFAdd), rightIRAddress(IRRAdd),
        leftIRShut(IRLShut), frontIRShut(IRFShut), rightIRShut(IRRShut),
        leftVive(viveL), rightVive(viveR)
    {
      robot = this;
      servo.attach(servoPin);
    }
    void init();
    void update();
    void setPID(float newKPL, float newKIL, float newKDL, float newKPR, float newKIR, float newKDR) {
      kPL = newKPL; kIL = newKIL; kDL = newKDL;
      kPR = newKPR, kIR = newKIR; kDR = newKDR;
    }
    Pose getPose();

    uint16_t getLeftDistance() { getDistance(leftIR); }
    uint16_t getRightDistance() { getDistance(rightIR); }
    uint16_t getFrontDistance() { getDistance(frontIR); }
    
    void drive(int left, int right);
    void fullSend(int left, int right);
    void sweep();
};

#endif ROBOT_H_
