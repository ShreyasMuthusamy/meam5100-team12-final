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

void CommandHandler::handleCommand(String &command) {
  if (command == "followWall") {
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
