#include "commands.h"

void CommandHandler::handleWallFollowing() {
  static unsigned long stateStartTime = 0;
  unsigned long currentMillis = millis();
  
  static enum { FOLLOW_WALL, REVERSE, TURN_LEFT1, TURN_LEFT2, TURN_LEFT3, TURN_RIGHT1, TURN_RIGHT2, TURN_RIGHT3, CHECK, AVOID_WALL } state = FOLLOW_WALL;
  // Declare variables outside switch to avoid scope issues
  float yd = 60.0; // Desired distance to the wall
  float dyd = 10.0;
  int vAvg = 30;
  switch (state) {
    case FOLLOW_WALL:
      // Forward logic for a set duration
      m_robot->fullSend(vAvg, vAvg);
      if (currentMillis - stateStartTime >= 800) { stateStartTime = currentMillis; state = CHECK; }
      break;
    case REVERSE:
      // Reverse logic for a brief time
      m_robot->fullSend(-vAvg, -vAvg);
      if (currentMillis - stateStartTime >= 300) { stateStartTime = currentMillis; state = AVOID_WALL; }
      break;
    case AVOID_WALL:
      // Turn logic for a set duration
      m_robot->fullSend(vAvg, -vAvg);
      if (currentMillis - stateStartTime >= 500) { stateStartTime = currentMillis; state = CHECK; }
      break;
    case TURN_LEFT1:
      // Turn logic for a set duration
      m_robot->fullSend(-vAvg/4, vAvg/4);
      if (currentMillis - stateStartTime >= 150) { stateStartTime = currentMillis; state = TURN_LEFT2; }
      break;
    case TURN_LEFT2:
      // Turn logic for a set duration
      m_robot->fullSend(vAvg, vAvg);
      if (currentMillis - stateStartTime >= 200) { stateStartTime = currentMillis; state = TURN_LEFT3; }
      break;
    case TURN_LEFT3:
      // Turn logic for a set duration
      m_robot->fullSend(vAvg/4, -vAvg/4);
      if (currentMillis - stateStartTime >= 150) { stateStartTime = currentMillis; state = CHECK; }
      break;
    case TURN_RIGHT1:
      // Turn logic for a set duration
      m_robot->fullSend(vAvg/4, -vAvg/4);
      if (currentMillis - stateStartTime >= 200) { stateStartTime = currentMillis; state = TURN_RIGHT2; }
      break;
    case TURN_RIGHT2:
      // Turn logic for a set duration
      m_robot->fullSend(vAvg, vAvg);
      if (currentMillis - stateStartTime >= 200) { stateStartTime = currentMillis; state = TURN_RIGHT3; }
      break;
    case TURN_RIGHT3:
      // Turn logic for a set duration
      m_robot->fullSend(-vAvg/4, vAvg/2);
      if (currentMillis - stateStartTime >= 150) { stateStartTime = currentMillis; state = CHECK; }
      break;
    case CHECK:
      m_robot->fullSend(0, 0);
      if (currentMillis - stateStartTime >= 500) {
        stateStartTime = currentMillis;
        int f = m_robot->getFrontDistance();
        if (f >= 10 && f <= 120) {
          state = REVERSE;
        } else {
          int y = m_robot->getLeftDistance();
          if (y > yd + dyd) { state = TURN_RIGHT1; }
          else if (y > yd - dyd && y <= yd + dyd) { state = FOLLOW_WALL; }
          else if (y < yd - dyd) { state = TURN_LEFT1; }
        }
      }
      break;
    default:
      m_robot->fullSend(0, 0);
      break;
  }
}

void CommandHandler::handleAutoAttackRight() {
  static unsigned long lastUpdate = 0;
  static unsigned long stateStartTime = 0;
  const unsigned long updateInterval = 50; // 50 ms update interval
  unsigned long currentMillis = millis();

  static enum { FOLLOW_WALL, REVERSE, TURN } state = FOLLOW_WALL;
  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;
    // Declare variables outside switch to avoid scope issues
    float y = m_robot->getLeftDistance() / 10.0; // Distance to the wall in cm
    float yd = 10; // Desired distance to the wall
    float kSide = 0.2;
    float kQ = 0.005;
    int vAvg = 30;
    int sgn = -1;
    float u = 0; // Control effort
    int uLeft = 0, uRight = 0;
    // Front distance check
    int f = m_robot->getFrontDistance();
    if (state == FOLLOW_WALL && f >= 10 && f <= 120) {
      state = REVERSE;
      stateStartTime = currentMillis;
    }
    switch (state) {
      case FOLLOW_WALL:
        // Wall-following logic
        u = kSide * sgn * (y - yd) - kQ * sgn * m_robot->getAngle();
        uLeft = round(vAvg + u);
        uRight = round(vAvg - u);
        m_robot->fullSend(uLeft, uRight+3);
        Serial.printf("Distance from wall: %.2f\n", y);
        break;
      case REVERSE:
        // Reverse logic for a brief time
        m_robot->fullSend(-vAvg/2, -vAvg);
        if (currentMillis - stateStartTime >= 400) {
            state = TURN;
            stateStartTime = currentMillis;
        }
        break;
      case TURN:
        // Turn logic for a set duration
        m_robot->fullSend(vAvg, -vAvg);
        if (currentMillis - stateStartTime >= 200) {
            state = FOLLOW_WALL;
            m_robot->clearEncoders();
        }
        break;
    }
  }
}

void CommandHandler::handleTeleop() {
  if (m_currControl == "front") {
    m_robot->fullSend(80, 80);
  } else if (m_currControl == "left") {
    m_robot->fullSend(-30, 30);
  } else if (m_currControl == "right") {
    m_robot->fullSend(30, -30);
  } else if (m_currControl == "back") {
    m_robot->fullSend(-80, -80);
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
