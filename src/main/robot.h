#ifndef ROBOT_H_
#define ROBOT_H_

#include <arduino.h>

// Motor LEDC values and PID frame rate
#define MOTOR_RES_BITS 12
#define MOTOR_RES ((1 << MOTOR_RES_BITS) - 1)
#define MOTOR_FREQ 100
#define FRAME_RATE 50

// Set a cap on the maximum integral error
#define MAX_ERR_I 60

class Robot {
  private:
    static Robot* robot;

    int leftMotorPWM, leftMotorDir, leftEncoderPin;
    int rightMotorPWM, rightMotorDir, rightEncoderPin;
    int servoPin;

    long leftEncoderCounts, rightEncoderCounts;
    byte leftFwd, rightFwd;
    
    void updateLeftEncoder() { leftEncoderCounts += leftFwd ? 1 : -1; }
    void updateRightEncoder() { rightEncoderCounts += rightFwd ? 1 : -1; }

    static void IRAM_ATTR handleLeftEncoderInterrupt();
    static void IRAM_ATTR handleRightEncoderInterrupt();

    float kP, kI, kD;
    // int kPL, kIL, kDL;
    // int kPR, kIR, kDR;
    int vLeft, vRight;

    int solvePIDLeft(int current, int setpoint);
    int solvePIDRight(int current, int setpoint);

    void setupMotors();
    void setupEncoders();

  public:
    Robot(int PWML, int DIRL, int ENCL, int PWMR, int DIRR, int ENCR, int servo):
      leftMotorPWM(PWML), leftMotorDir(DIRL), leftEncoderPin(ENCL),
      rightMotorPWM(PWMR), rightMotorDir(DIRR), rightEncoderPin(ENCR),
      servoPin(servo) { robot = this; }
    void init();
    void setPID(float newKP, float newKI, float newKD) { kP = newKP; kI = newKI; kD = newKD; }
    void update();
    void drive(int left, int right);
};

#endif ROBOT_H_
