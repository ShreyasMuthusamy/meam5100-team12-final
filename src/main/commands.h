#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <arduino.h>
#include "robot.h"
#include "geometry.h"

#define R_THRESHOLD 50

class CommandHandler {
  private:
    String m_currCommand, m_currControl;
    Robot* m_robot;

  public:
    CommandHandler(Robot* robot) : m_robot(robot) {}
    void setCommand(String &command) { m_currCommand = command; }
    String getCommand() { return m_currCommand; }
    void setControl(String &control) { m_currControl = control; }

    void handleCommand(String &command);
    void handleWallFollowing();
    void handleAutoAttackLeft() {}
    void handleAutoAttackCenter() {}
    void handleAutoAttackRight();
    void handleTeleop();

    void run();
};

#endif COMMANDS_H_
