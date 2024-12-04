#include "commands.h"

void FollowWall::initialize() {
  if (robot->getLeftDistance() < robot->getRightDistance()) {
    wallToFollow = LEFT_WALL;
  } else {
    wallToFollow = RIGHT_WALL;
  }
}

void FollowWall::execute() {
  int sgn = wallToFollow ? 1 : -1;
  float y = wallToFollow ? robot->getRightDistance() : robot->getLeftDistance();

  float u = kSide * sgn * (y - yd);
  if (robot->getFrontDistance() < fd) {
    u += kFront * sgn * (robot->getFrontDistance() - fd);
  }

  robot->drive(vAvg+u, vAvg-u);
}

void AutoAttack::initialize() {
  Pose p3;
  switch (target) {
    case LEFT_TARG:
      // TODO: Locate left target
      break;
    case CENTER_TARG:
      // TODO: Locate center target
      break;
    case RIGHT_TARG:
      // TODO: Locate right target
      break;
    default:
      p3 = robot->getPose();
      break;
  }

  traj = new Trajectory(robot->getPose(), p3, 10);
}

void AutoAttack::execute() {
  Pose currPose = robot->getPose();
  float k = traj->getCurvature(currPose);

  float u = 2 * k;
  robot->drive(vAvg+u, vAvg-u);
}

void Scheduler::schedule(Command* command) {
  int priority = command->getPriority();
  auto pos = commands.begin();
  
  switch (priority) {
    case IMMEDIATE:
      commands.insert(std::next(pos, numImmediate), command);
      numImmediate++;
      break;
    case TELEOP:
      commands.insert(std::next(pos, numImmediate+numTeleop), command);
      numTeleop++;
      break;
    case AUTO:
    default:
      commands.insert(commands.end(), command);
      break;
  }
}

void Scheduler::schedule(String command, Robot* robot) {
  if (command == "wallFollow") {
    FollowWall c(robot);
    schedule(&c);
  } else if (command == "autoAttackLeft") {
    AutoAttack c(robot, LEFT_TARG);
    schedule(&c);
  } else if (command == "autoAttackCenter") {
    AutoAttack c(robot, CENTER_TARG);
    schedule(&c);
  } else if (command == "autoAttackRight") {
    AutoAttack c(robot, RIGHT_TARG);
    schedule(&c);
  } else if (command == "stop") {
    EmergencyStop c(robot);
    schedule(&c);
  }
}

void Scheduler::run() {
  if (!initialized) {
    commands.front()->initialize();
    initialized = true;
    finished = false;
    return;
  }

  if (!commands.front()->finished()) {
    commands.front()->execute();
    finished = false;
    return;
  }

  static unsigned long currTime = millis();
  if (!finished && commands.front()->finished()) {
    currTime = millis();
  }

  if (millis() - currTime > REST_MILLIS) {
    commands.erase(commands.begin());
    initialized = false;
  } else {
    commands.front()->stop();
  }

  finished = commands.front()->finished();
}
