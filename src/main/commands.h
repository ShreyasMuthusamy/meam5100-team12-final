#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <arduino.h>
#include "robot.h"

#define IMMEDIATE 0
#define AUTO 1
#define TELEOP 2

class Command {
  private:
    String name;
    int priority;
    Robot* robot;
  
  public:
    Command(String _name, int _priority, Robot* _robot): name(_name), priority(_priority), robot(_robot) {}

    void initialize();
    void execute();
    bool finished();
    void stop();

    String getName() { return name; }
    int getPriority() { return priority; }
};

class Scheduler {
  private:
    Robot* robot;
    std::vector<Command> commands;
    int numImmediate = 0, numAuto = 0;
  
  public:
    Scheduler(Robot* _robot): robot(_robot) {}
    void schedule(Command command);
    void schedule(String command);
    bool update();
    Command getCurrentCommand() { return commands.front(); }
};

#endif COMMANDS_H_
