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
  // if (robot->getFrontDistance() < fd) {
  //   u += kFront * sgn * (robot->getFrontDistance() - fd);
  // }

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
