#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <arduino.h>
#include "robot.h"

#define IMMEDIATE 0
#define AUTO 1
#define TELEOP 2

class Command {
  private:
    String _name;
    int _priority;
    Robot* _robot;
  
  public:
    Command(String name, int priority, Robot* robot): _name(name), _priority(priority), _robot(robot) {}

    void initialize();
    void execute();
    bool finished();
    void stop();

    String getName() { return _name; }
    int getPriority() { return _priority; }
};

class Scheduler {
  private:
    Robot* _robot;
    std::vector<Command> commands;
    int numImmediate = 0, numAuto = 0;
  
  public:
    Scheduler(Robot* robot): _robot(robot) {}
    void schedule(Command command);
    void schedule(String command);
    bool update();
    Command getCurrentCommand() { return commands.front(); }
};

#endif COMMANDS_H_