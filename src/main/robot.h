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
#define FRAME_RATE 50

// SDA and SCL pins for I2C
#define SDA_PIN 4
#define SCL_PIN 5

// Set a cap on the maximum integral error
#define MAX_ERR_I 60

class Robot {
  private:
    static Robot* robot;

    int leftMotorPWM, leftMotorDir, leftEncoderPin;
    int rightMotorPWM, rightMotorDir, rightEncoderPin;
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
    
    void updateLeftEncoder() { leftEncoderCounts += leftFwd ? 1 : -1; }
    void updateRightEncoder() { rightEncoderCounts += rightFwd ? 1 : -1; }

    static void IRAM_ATTR handleLeftEncoderInterrupt();
    static void IRAM_ATTR handleRightEncoderInterrupt();

    float kP, kI, kD;
    int vLeft, vRight;

    int solvePIDLeft(int current, int setpoint);
    int solvePIDRight(int current, int setpoint);

    uint16_t getDistance(Adafruit_VL53L0X tof);

    void setupMotors();
    void setupEncoders();
    void setupSensors();

  public:
    Robot(
      int PWML, int DIRL, int ENCL,
      int PWMR, int DIRR, int ENCR,
      int IRLAdd, int IRFAdd, int IRRAdd,
      int IRLShut, int IRFShut, int IRRShut,
      int viveL, int viveR, int servoPin
    ):
        leftMotorPWM(PWML), leftMotorDir(DIRL), leftEncoderPin(ENCL),
        rightMotorPWM(PWMR), rightMotorDir(DIRR), rightEncoderPin(ENCR),
        leftIRAddress(IRLAdd), frontIRAddress(IRFAdd), rightIRAddress(IRRAdd),
        leftIRShut(IRLShut), frontIRShut(IRFShut), rightIRShut(IRRShut),
        leftVive(viveL), rightVive(viveR)
    {
      robot = this;
      servo.attach(servoPin);
    }
    void init();
    void setPID(float newKP, float newKI, float newKD) { kP = newKP; kI = newKI; kD = newKD; }
    Pose getPose();

    uint16_t getLeftDistance() { getDistance(leftIR); }
    uint16_t getRightDistance() { getDistance(rightIR); }
    uint16_t getFrontDistance() { getDistance(frontIR); }
    
    void drive(int left, int right);
    void sweep();
};

#endif ROBOT_H_
