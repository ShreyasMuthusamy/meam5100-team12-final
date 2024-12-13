#include "commands.h"

void CommandHandler::handleWallFollowing() {
  int vAvg = 20;
  float yd = 10;
  float kSide = 0.2;
  float kQ = 0.01;
  int sgn = -1;
  // float y = m_robot->getLeftDistance() / 10.0;
  float y = 0;

  int uLeft, uRight;

  // if (m_robot->getFrontDistance() > 200) {
  //   float u = kSide * sgn * (y - yd) + kQ * sgn * m_robot->getAngle();
  //   uLeft = round(vAvg + u);
  //   uRight = round(vAvg - u);
  // } else {
  //   m_robot->clearEncoders();
  //   uLeft = 50;
  //   uRight = -50;
  // }
  m_robot->fullSend(uLeft, uRight);
  // Serial.printf("Distance to wall: %.2f, Attempted control: Left = %d, Right = %d\n", y, uLeft, uRight);
}

void CommandHandler::handleAutoAttackRight() {
  Pose waypoint; waypoint.x = 3950; waypoint.y = 2650;
  Pose robotPose = m_robot->getPose();
  double desiredQ = atan2(waypoint.y - robotPose.y, waypoint.x - robotPose.x);
  double errQ = desiredQ - robotPose.theta;
  if (errQ > PI) {
    errQ -= 2 * PI;
  } else if (E.errQ < -PI) {
    errQ += 2 * PI;
  }

  if (sqrt(pow(robotPose.y - waypoint.y, 2) + pow(robotPose.x - waypoint.x, 2)) > R_THRESHOLD) {
    double pwmDel = constrain(3*errQ, -10, 10);
    double pwmAvg = 50;

    m_robot->fullSend(round(pwmAvg-pwmDel), round(pwmAvg+pwmDel));
  } else {
    m_robot->fullSend(0, 0);
  }
}

void CommandHandler::handleTeleop() {
  if (m_currControl == "front") {
    m_robot->fullSend(50, 50);
  } else if (m_currControl == "left") {
    m_robot->fullSend(-50, 50);
  } else if (m_currControl == "right") {
    m_robot->fullSend(50, -50);
  } else if (m_currControl == "back") {
    m_robot->fullSend(-50, -50);
  } else if (m_currControl == "attack") {
    m_robot->attack();
  } else {
    m_robot->fullSend(0, 0);
  }
}

void CommandHandler::handleCommand(String &command) {
  if (command == "wallFollow") {
    handleWallFollowing();
  } else if (command == "autoAttackLeft") {
    handleAutoAttackLeft();
  } else if (command == "autoAttackCenter") {
    handleAutoAttackCenter();
  } else if (command == "autoAttackRight") {
    handleAutoAttackRight();
  } else if (command == "teleop") {
    handleTeleop();
  } else {
    m_robot->fullSend(0, 0);
  }
}

void CommandHandler::run() {
  handleCommand(m_currCommand);
}
