#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <arduino.h>
#include "robot.h"
#include "geometry.h"

#define IMMEDIATE 0
#define TELEOP 1
#define AUTO 2

#define LEFT_WALL 0
#define RIGHT_WALL 1

#define LEFT_TARG 0
#define CENTER_TARG 1
#define RIGHT_TARG 2

#define REST_MILLIS 1000

class Command {
  protected:
    int priority;
    Robot* robot;
  
  public:
    Command(int _priority, Robot* _robot): priority(_priority), robot(_robot) {}

    virtual void initialize() = 0;
    virtual void execute() = 0;
    virtual bool finished() = 0;
    virtual void stop() = 0;

    int getPriority() { return priority; }
};

class EmergencyStop : public Command {
  public:
    EmergencyStop(Robot* _robot): Command(IMMEDIATE, _robot) {}

    void initialize() {}
    void execute() { robot->drive(0, 0); }
    bool finished() { return false; }
    void stop() { robot->drive(0, 0); }
};

class FollowWall : public Command {
  private:
    float kSide = 2;
    float kFront = 10;
    float yd = 10;
    float fd = 20;
    float vAvg = 20;
    int wallToFollow;

  public:
    FollowWall(Robot* _robot): Command(AUTO, _robot) {}

    void initialize() override;
    void execute() override;
    bool finished() override { return false; }
    void stop() override { robot->drive(0, 0); }
};

class AutoAttack : public Command {
  private:
    int target;
    int vAvg = 30;
    Trajectory* traj = nullptr;
  
  public:
    AutoAttack(Robot* _robot, int _target): Command(AUTO, _robot), target(_target) {}
    ~AutoAttack() { if (traj) { delete traj; } }

    void initialize() override;
    void execute() override;
    bool finished() override { return traj->finished(robot->getPose()); }
    void stop() override { robot->drive(0, 0); }
};

class Scheduler {
  private:
    Robot* robot;
    std::vector<Command*> commands;
    int numImmediate = 0, numTeleop = 0;
    bool initialized = false, finished = false;
  
  public:
    Scheduler(Robot* _robot): robot(_robot) {}
    void schedule(Command* command);
    void schedule(String command, Robot* robot);
    void run();
    Command* getCurrentCommand() { return commands.front(); }
};

#endif COMMANDS_H_
